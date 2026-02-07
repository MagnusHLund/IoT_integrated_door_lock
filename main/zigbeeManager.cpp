#include "zigbeeManager.hpp"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_zigbee_core.h"
#include "ha/esp_zigbee_ha_standard.h"
#include "zcl/esp_zigbee_zcl_basic.h"
#include "zcl_utility.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "config.hpp"
#include <cstring>

static const char* TAG = "ZigbeeManager";

#define INSTALLCODE_POLICY_ENABLE false
#define ESP_ZB_PRIMARY_CHANNEL_MASK ESP_ZB_TRANSCEIVER_ALL_CHANNELS_MASK

static char kManufacturerName[] = "\x0A" "MagnusLund";
static char kModelIdentifier[] = "\x0A" "DoorLockC6";
static char kProductLabel[] = "\x0E" "ZB Door Lock";
static char kSwBuildId[] = "\x06" "1.0.0";

#define ESP_ZB_DEFAULT_RADIO_CONFIG()              \
    {                                               \
        .radio_mode = ZB_RADIO_MODE_NATIVE,         \
    }

#define ESP_ZB_DEFAULT_HOST_CONFIG()               \
    {                                               \
        .host_connection_mode = ZB_HOST_CONNECTION_MODE_NONE, \
    }

ZigbeeManager* ZigbeeManager::instance_ = nullptr;
static bool s_factory_reset_requested = false;

ZigbeeManager::ZigbeeManager() : connected(false) {
    instance_ = this;
}

ZigbeeManager::~ZigbeeManager() {
    instance_ = nullptr;
}

static void bdb_start_top_level_commissioning_cb(uint8_t mode_mask)
{
    ESP_ERROR_CHECK(esp_zb_bdb_start_top_level_commissioning(mode_mask));
}

void esp_zb_app_signal_handler(esp_zb_app_signal_t *signal_struct)
{
    uint32_t *p_sg_p = signal_struct->p_app_signal;
    esp_err_t err_status = signal_struct->esp_err_status;
    esp_zb_app_signal_type_t sig_type = static_cast<esp_zb_app_signal_type_t>(*p_sg_p);

    switch (sig_type) {
    case ESP_ZB_ZDO_SIGNAL_SKIP_STARTUP:
        ESP_LOGI(TAG, "Initialize Zigbee stack");
        esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_INITIALIZATION);
        break;
    case ESP_ZB_BDB_SIGNAL_DEVICE_FIRST_START:
    case ESP_ZB_BDB_SIGNAL_DEVICE_REBOOT:
        if (err_status == ESP_OK) {
            ESP_LOGI(TAG, "Device started, factory-new: %s", esp_zb_bdb_is_factory_new() ? "yes" : "no");
            if (esp_zb_bdb_is_factory_new()) {
                ESP_LOGI(TAG, "Start network steering");
                esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_NETWORK_STEERING);
            }
        } else {
            ESP_LOGW(TAG, "Failed to initialize Zigbee stack (status: %s)", esp_err_to_name(err_status));
        }
        break;
    case ESP_ZB_BDB_SIGNAL_STEERING:
        if (err_status == ESP_OK) {
            esp_zb_ieee_addr_t extended_pan_id;
            esp_zb_get_extended_pan_id(extended_pan_id);
            ESP_LOGI(TAG, "Joined network successfully (Extended PAN ID: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x)",
                     extended_pan_id[7], extended_pan_id[6], extended_pan_id[5], extended_pan_id[4],
                     extended_pan_id[3], extended_pan_id[2], extended_pan_id[1], extended_pan_id[0]);
            if (ZigbeeManager::getInstance()) {
                ZigbeeManager::getInstance()->setConnected(true);
            }
        } else {
            ESP_LOGI(TAG, "Network steering failed (status: %s)", esp_err_to_name(err_status));
            esp_zb_scheduler_alarm(bdb_start_top_level_commissioning_cb,
                                  ESP_ZB_BDB_MODE_NETWORK_STEERING, 1000);
        }
        break;
    default:
        ESP_LOGI(TAG, "ZDO signal: %s (0x%x), status: %s",
                 esp_zb_zdo_signal_to_string(sig_type), sig_type, esp_err_to_name(err_status));
        break;
    }
}

static esp_err_t zb_attribute_handler(const esp_zb_zcl_set_attr_value_message_t *message)
{
    ESP_RETURN_ON_FALSE(message, ESP_FAIL, TAG, "Empty message");
    ESP_RETURN_ON_FALSE(message->info.status == ESP_ZB_ZCL_STATUS_SUCCESS, ESP_ERR_INVALID_ARG, TAG,
                        "Received message: error status(%d)", message->info.status);

    if (message->info.dst_endpoint == ZigbeeManager::ENDPOINT &&
        message->info.cluster == ESP_ZB_ZCL_CLUSTER_ID_ON_OFF &&
        message->attribute.id == ESP_ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID &&
        message->attribute.data.type == ESP_ZB_ZCL_ATTR_TYPE_BOOL) {
        bool on = message->attribute.data.value ? *(bool *)message->attribute.data.value : false;
        if (ZigbeeManager::getInstance()) {
            ZigbeeManager::getInstance()->processCommand(on ? "LOCK" : "UNLOCK");
        }
    }

    return ESP_OK;
}

static esp_err_t zb_action_handler(esp_zb_core_action_callback_id_t callback_id, const void *message)
{
    esp_err_t ret = ESP_OK;
    switch (callback_id) {
    case ESP_ZB_CORE_SET_ATTR_VALUE_CB_ID:
        ret = zb_attribute_handler((const esp_zb_zcl_set_attr_value_message_t *)message);
        break;
    default:
        ESP_LOGW(TAG, "Receive Zigbee action(0x%x) callback", callback_id);
        break;
    }
    return ret;
}

static void set_basic_device_info(void)
{
    uint8_t hw_version = 1;
    uint8_t power_source = ESP_ZB_ZCL_BASIC_POWER_SOURCE_MAINS_SINGLE_PHASE;
    bool device_enabled = true;

    esp_zb_zcl_set_attribute_val(ZigbeeManager::ENDPOINT,
                                 ESP_ZB_ZCL_CLUSTER_ID_BASIC,
                                 ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
                                 ESP_ZB_ZCL_ATTR_BASIC_MANUFACTURER_NAME_ID,
                                 kManufacturerName,
                                 false);
    esp_zb_zcl_set_attribute_val(ZigbeeManager::ENDPOINT,
                                 ESP_ZB_ZCL_CLUSTER_ID_BASIC,
                                 ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
                                 ESP_ZB_ZCL_ATTR_BASIC_MODEL_IDENTIFIER_ID,
                                 kModelIdentifier,
                                 false);
    esp_zb_zcl_set_attribute_val(ZigbeeManager::ENDPOINT,
                                 ESP_ZB_ZCL_CLUSTER_ID_BASIC,
                                 ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
                                 ESP_ZB_ZCL_ATTR_BASIC_PRODUCT_LABEL_ID,
                                 kProductLabel,
                                 false);
    esp_zb_zcl_set_attribute_val(ZigbeeManager::ENDPOINT,
                                 ESP_ZB_ZCL_CLUSTER_ID_BASIC,
                                 ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
                                 ESP_ZB_ZCL_ATTR_BASIC_SW_BUILD_ID,
                                 kSwBuildId,
                                 false);
    esp_zb_zcl_set_attribute_val(ZigbeeManager::ENDPOINT,
                                 ESP_ZB_ZCL_CLUSTER_ID_BASIC,
                                 ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
                                 ESP_ZB_ZCL_ATTR_BASIC_HW_VERSION_ID,
                                 &hw_version,
                                 false);
    esp_zb_zcl_set_attribute_val(ZigbeeManager::ENDPOINT,
                                 ESP_ZB_ZCL_CLUSTER_ID_BASIC,
                                 ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
                                 ESP_ZB_ZCL_ATTR_BASIC_POWER_SOURCE_ID,
                                 &power_source,
                                 false);
    esp_zb_zcl_set_attribute_val(ZigbeeManager::ENDPOINT,
                                 ESP_ZB_ZCL_CLUSTER_ID_BASIC,
                                 ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
                                 ESP_ZB_ZCL_ATTR_BASIC_DEVICE_ENABLED_ID,
                                 &device_enabled,
                                 false);
}

static void zigbee_task(void *pvParameters)
{
    (void)pvParameters;

    esp_zb_cfg_t zb_nwk_cfg = {};
    zb_nwk_cfg.esp_zb_role = ESP_ZB_DEVICE_TYPE_ROUTER;
    zb_nwk_cfg.install_code_policy = INSTALLCODE_POLICY_ENABLE;
    zb_nwk_cfg.nwk_cfg.zczr_cfg.max_children = 10;
    esp_zb_init(&zb_nwk_cfg);

    if (s_factory_reset_requested) {
        ESP_LOGW(TAG, "Factory reset requested, erasing Zigbee storage");
        esp_zb_factory_reset();
        vTaskDelete(NULL);
    }

    esp_zb_on_off_light_cfg_t light_cfg = ESP_ZB_DEFAULT_ON_OFF_LIGHT_CONFIG();
    esp_zb_ep_list_t *on_off_ep = esp_zb_on_off_light_ep_create(ZigbeeManager::ENDPOINT, &light_cfg);

    zcl_basic_manufacturer_info_t info = {
        .manufacturer_name = kManufacturerName,
        .model_identifier = kModelIdentifier,
    };
    esp_zcl_utility_add_ep_basic_manufacturer_info(on_off_ep, ZigbeeManager::ENDPOINT, &info);

    esp_zb_device_register(on_off_ep);
    set_basic_device_info();
    esp_zb_core_action_handler_register(zb_action_handler);
    esp_zb_set_primary_network_channel_set(ESP_ZB_PRIMARY_CHANNEL_MASK);
    ESP_ERROR_CHECK(esp_zb_start(false));
    esp_zb_stack_main_loop();
}

void ZigbeeManager::start() {
    ESP_LOGI(TAG, "Initializing Zigbee stack");

    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = (1ULL << FACTORY_RESET_PIN);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);

    const int check_interval_ms = 100;
    int remaining_ms = 5;
    s_factory_reset_requested = true;
    while (remaining_ms > 0) {
        if (gpio_get_level(static_cast<gpio_num_t>(FACTORY_RESET_PIN)) != 0) {
            s_factory_reset_requested = false;
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(check_interval_ms));
        remaining_ms -= check_interval_ms;
    }
    if (s_factory_reset_requested) {
        ESP_LOGW(TAG, "Factory reset pin held, will reset Zigbee storage");
    }

    esp_zb_platform_config_t config = {};
    config.radio_config.radio_mode = ZB_RADIO_MODE_NATIVE;
    config.host_config.host_connection_mode = ZB_HOST_CONNECTION_MODE_NONE;
    ESP_ERROR_CHECK(esp_zb_platform_config(&config));

    xTaskCreate(zigbee_task, "Zigbee_main", 4096, NULL, 5, NULL);
}

void ZigbeeManager::awaitConnectivity() {
    int attempts = 0;
    const int max_attempts = 60;

    while (!connected && attempts < max_attempts) {
        ESP_LOGI(TAG, "Waiting for Zigbee network connection... (%d/%d)", attempts + 1, max_attempts);
        vTaskDelay(pdMS_TO_TICKS(1000));
        attempts++;
    }

    if (connected) {
        ESP_LOGI(TAG, "Connected to Zigbee network");
    } else {
        ESP_LOGW(TAG, "Zigbee connection timeout after %d seconds", max_attempts);
    }
}

bool ZigbeeManager::isConnected() const {
    return connected;
}

void ZigbeeManager::setConnected(bool state) {
    connected = state;
}

void ZigbeeManager::publishLockState(const char* state) {
    if (!connected) {
        ESP_LOGW(TAG, "Cannot publish, Zigbee not connected");
        return;
    }

    bool on = (strcmp(state, "LOCKED") == 0);
    esp_zb_zcl_set_attribute_val(ENDPOINT,
                                 ESP_ZB_ZCL_CLUSTER_ID_ON_OFF,
                                 ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
                                 ESP_ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID,
                                 &on,
                                 false);

    ESP_LOGI(TAG, "Lock state updated: %s", state);
}

void ZigbeeManager::setCommandCallback(std::function<void(char*)> callback) {
    commandCallback = callback;
}

void ZigbeeManager::processCommand(const char* command) {
    if (commandCallback && command) {
        commandCallback(const_cast<char*>(command));
    }
}

ZigbeeManager* ZigbeeManager::getInstance() {
    return instance_;
}
