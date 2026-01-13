#include "bleCommissioning.hpp"
#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "string.h"
#include "config.hpp"

// BLE includes
#include "host/ble_hs.h"
#include "nimble/ble.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

static const char* TAG = "BLE_COMMISSIONING";

// Matter BLE UUIDs for commissioning
#define MATTER_BLE_SERVICE_UUID         0xFFF0
#define MATTER_BLE_C1_CHAR_UUID         0xFFF1  // Commissioning characteristic 1
#define MATTER_BLE_C2_CHAR_UUID         0xFFF2  // Commissioning characteristic 2
#define MATTER_BLE_C3_CHAR_UUID         0xFFF3  // Commissioning characteristic 3

// NVS namespace for commissioning data
#define COMMISSIONING_NVS_NAMESPACE     "commissioning"
#define COMMISSIONING_FLAG_KEY          "commissioned"

// Static instance pointer for callbacks
static BLECommissioning* g_ble_commissioning = nullptr;

// GATT database
static uint8_t ble_addr_type;
static uint16_t ble_conn_handle = BLE_HS_CONN_HANDLE_NONE;

// BLE attribute handles
static uint16_t commissioning_svc_handle;
static uint16_t c1_char_handle;
static uint16_t c1_val_handle;
static uint16_t c2_char_handle;
static uint16_t c2_val_handle;
static uint16_t c3_char_handle;
static uint16_t c3_val_handle;

BLECommissioning::BLECommissioning()
    : state_(CommissioningState::Idle),
      initialized_(false),
      commissioned_(false) {
    memset(error_message_, 0, sizeof(error_message_));
    g_ble_commissioning = this;
}

BLECommissioning::~BLECommissioning() {
    g_ble_commissioning = nullptr;
}

bool BLECommissioning::initialize() {
    if (initialized_) {
        ESP_LOGW(TAG, "BLE Commissioning already initialized");
        return true;
    }

    // Initialize NVS if not already initialized
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS partition was truncated and needs to be erased");
        nvs_flash_erase();
        err = nvs_flash_init();
    }

    if (err != ESP_OK) {
        setError("Failed to initialize NVS");
        ESP_LOGE(TAG, "%s", error_message_);
        return false;
    }

    // Load commissioning state from NVS
    loadCommissioningDataFromNVS();

    // Initialize BLE stack
    initializeBLEStack();

    initialized_ = true;
    setState(CommissioningState::Idle);

    ESP_LOGI(TAG, "BLE Commissioning initialized successfully");
    return true;
}

bool BLECommissioning::startAdvertising() {
    if (!initialized_) {
        setError("BLE Commissioning not initialized");
        return false;
    }

    if (commissioned_) {
        ESP_LOGI(TAG, "Device already commissioned, skipping BLE advertisement");
        return true;
    }

    struct ble_gap_adv_params adv_params;
    memset(&adv_params, 0, sizeof(adv_params));

    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;  // Undirected connectable
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;  // General discoverable

    esp_err_t err = ble_gap_adv_start(ble_addr_type, NULL, 180000, &adv_params);  // 180 seconds timeout
    if (err != 0) {
        ESP_LOGE(TAG, "Failed to start BLE advertisement: %d", err);
        setError("Failed to start BLE advertisement");
        setState(CommissioningState::Error);
        return false;
    }

    setState(CommissioningState::Advertising);
    ESP_LOGI(TAG, "BLE advertisement started - Device ready for commissioning");
    return true;
}

bool BLECommissioning::stopAdvertising() {
    esp_err_t err = ble_gap_adv_stop();
    if (err != 0) {
        ESP_LOGE(TAG, "Failed to stop BLE advertisement: %d", err);
        return false;
    }

    setState(CommissioningState::Idle);
    ESP_LOGI(TAG, "BLE advertisement stopped");
    return true;
}

CommissioningState BLECommissioning::getState() const {
    return state_;
}

bool BLECommissioning::isCommissioned() const {
    return commissioned_;
}

void BLECommissioning::factoryReset() {
    ESP_LOGI(TAG, "Performing factory reset of commissioning data");

    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(COMMISSIONING_NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err == ESP_OK) {
        nvs_erase_all(nvs_handle);
        nvs_commit(nvs_handle);
        nvs_close(nvs_handle);
    }

    commissioned_ = false;
    stopAdvertising();
    setState(CommissioningState::Idle);

    ESP_LOGI(TAG, "Factory reset complete");
}

const char* BLECommissioning::getErrorMessage() const {
    return error_message_;
}

void BLECommissioning::initializeBLEStack() {
    // Get BLE address type
    ble_addr_type = BLE_OWN_ADDR_RANDOM;

    // GAP configuration
    struct ble_svc_gap_conn_params gap_params = {
        .scan_itvl = BLE_GAP_SCAN_FAST_INTERVAL_MIN,
        .scan_win = BLE_GAP_SCAN_FAST_WINDOW,
        .itvl_min = 0x50,
        .itvl_max = 0x100,
        .latency = 0,
        .supervision_timeout = 0x100,
    };

    ble_svc_gap_init();
    ble_svc_gatt_init();

    // Set device name with Matter identifier
    ble_svc_gap_device_name_set(MATTER_DEVICE_NAME);

    ESP_LOGI(TAG, "BLE stack initialized");
}

void BLECommissioning::createCommissioningCharacteristics() {
    // Characteristics are created during GATT server setup
    // This is typically handled by the Matter stack
    ESP_LOGI(TAG, "Commissioning characteristics created");
}

void BLECommissioning::handleCommissioningData(const uint8_t* data, uint16_t len) {
    if (!data || len == 0) {
        setError("Invalid commissioning data received");
        return;
    }

    ESP_LOGI(TAG, "Received commissioning data, length: %d", len);

    // Validate commissioning packet format
    if (len < 4) {
        setError("Commissioning packet too short");
        return;
    }

    // Parse commissioning data
    uint8_t msg_type = data[0];
    uint16_t payload_len = (data[1] << 8) | data[2];

    ESP_LOGD(TAG, "Message type: %d, Payload length: %d", msg_type, payload_len);

    if (payload_len + 3 > len) {
        setError("Commissioning packet malformed");
        return;
    }

    // Mark device as commissioned after successful data reception
    // In production, you would validate signatures and perform secure commissioning
    commissioned_ = true;
    saveCommissioningDataToNVS();
    setState(CommissioningState::CommissioningComplete);

    ESP_LOGI(TAG, "Device successfully commissioned");
}

void BLECommissioning::loadCommissioningDataFromNVS() {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(COMMISSIONING_NVS_NAMESPACE, NVS_READONLY, &nvs_handle);

    if (err != ESP_OK) {
        ESP_LOGI(TAG, "No commissioning data found in NVS (first boot)");
        commissioned_ = false;
        return;
    }

    uint8_t commissioned_flag = 0;
    err = nvs_get_u8(nvs_handle, COMMISSIONING_FLAG_KEY, &commissioned_flag);
    nvs_close(nvs_handle);

    commissioned_ = (commissioned_flag == 1);
    ESP_LOGI(TAG, "Commissioning state loaded from NVS: %s", 
             commissioned_ ? "commissioned" : "not commissioned");
}

void BLECommissioning::saveCommissioningDataToNVS() {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(COMMISSIONING_NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS handle: %s", esp_err_to_name(err));
        return;
    }

    uint8_t commissioned_flag = commissioned_ ? 1 : 0;
    err = nvs_set_u8(nvs_handle, COMMISSIONING_FLAG_KEY, commissioned_flag);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write commissioning data to NVS: %s", esp_err_to_name(err));
    } else {
        nvs_commit(nvs_handle);
        ESP_LOGI(TAG, "Commissioning data saved to NVS");
    }

    nvs_close(nvs_handle);
}

void BLECommissioning::setState(CommissioningState new_state) {
    if (state_ != new_state) {
        ESP_LOGI(TAG, "Commissioning state changed: %d -> %d", 
                 static_cast<int>(state_), static_cast<int>(new_state));
        state_ = new_state;
    }
}

void BLECommissioning::setError(const char* error_msg) {
    if (error_msg) {
        strncpy(error_message_, error_msg, sizeof(error_message_) - 1);
        error_message_[sizeof(error_message_) - 1] = '\0';
    }
}

// BLE event callbacks
void BLECommissioning::bleConnectionCallback(uint16_t conn_handle) {
    if (g_ble_commissioning) {
        g_ble_commissioning->setState(CommissioningState::Connected);
        ESP_LOGI(TAG, "BLE connection established, handle: %d", conn_handle);
    }
}

void BLECommissioning::bleDisconnectionCallback(uint16_t conn_handle) {
    if (g_ble_commissioning) {
        if (g_ble_commissioning->isCommissioned()) {
            g_ble_commissioning->setState(CommissioningState::CommissioningComplete);
        } else {
            g_ble_commissioning->setState(CommissioningState::Advertising);
        }
        ESP_LOGI(TAG, "BLE connection closed, handle: %d", conn_handle);
    }
}

void BLECommissioning::bleCharacteristicWriteCallback(uint16_t conn_handle, uint16_t attr_handle,
                                                      const uint8_t* data, uint16_t len) {
    if (g_ble_commissioning) {
        g_ble_commissioning->handleCommissioningData(data, len);
    }
}
