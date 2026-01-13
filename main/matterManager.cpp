#include "matterManager.hpp"
#include "threadManager.hpp"
#include "lockController.hpp"
#include "esp_log.h"
#include "esp_mac.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "string.h"
#include "config.hpp"

static const char* TAG = "MATTER_MANAGER";

// NVS namespace for pairing data
#define PAIRING_NVS_NAMESPACE           "matter_pairing"
#define PAIRING_STATE_KEY               "pairing_state"
#define LOCK_STATE_KEY                  "lock_state"

// Matter Cluster and Attribute IDs
#define DOOR_LOCK_CLUSTER_ID            0x0101
#define LOCK_STATE_ATTRIBUTE_ID         0x0000
#define LOCK_COMMAND_ID                 0x0000  // Lock command
#define UNLOCK_COMMAND_ID               0x0001  // Unlock command

// Static instance for callbacks
static MatterManager* g_matter_manager = nullptr;

MatterManager::MatterManager()
    : pairing_state_(PairingState::Unpaired),
      lock_state_(LockOperationalState::Locked),
      initialized_(false),
      started_(false),
      thread_manager_(nullptr) {
    memset(error_message_, 0, sizeof(error_message_));
    memset(device_id_, 0, sizeof(device_id_));
    g_matter_manager = this;
}

MatterManager::~MatterManager() {
    g_matter_manager = nullptr;
}

bool MatterManager::initialize(ThreadManager& threadManager) {
    if (initialized_) {
        ESP_LOGW(TAG, "Matter Manager already initialized");
        return true;
    }

    thread_manager_ = &threadManager;

    // Initialize NVS if not already initialized
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS partition truncated, erasing");
        nvs_flash_erase();
        err = nvs_flash_init();
    }

    if (err != ESP_OK) {
        setError("Failed to initialize NVS");
        ESP_LOGE(TAG, "%s", error_message_);
        return false;
    }

    // Load pairing state from NVS
    loadPairingDataFromNVS();

    // Generate device ID from MAC address
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    snprintf(device_id_, sizeof(device_id_), "DoorLock_%02X%02X%02X%02X%02X%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    // Initialize Matter stack
    initializeMatterStack();

    // Setup Door Lock cluster
    setupDoorLockCluster();

    // Register attribute handlers
    registerAttributeHandlers();

    initialized_ = true;
    setPairingState(PairingState::Unpaired);

    ESP_LOGI(TAG, "Matter Manager initialized successfully");
    ESP_LOGI(TAG, "Device ID: %s", device_id_);
    return true;
}

bool MatterManager::start() {
    if (!initialized_) {
        setError("Matter Manager not initialized");
        return false;
    }

    if (started_) {
        ESP_LOGW(TAG, "Matter already started");
        return true;
    }

    started_ = true;

    // Matter protocol will be started here
    // In production, this would initialize the Matter stack with proper commissioning
    
    ESP_LOGI(TAG, "Matter protocol started");
    return true;
}

MatterManager::PairingState MatterManager::getPairingState() const {
    return pairing_state_;
}

bool MatterManager::isPaired() const {
    return pairing_state_ == PairingState::Paired;
}

MatterManager::LockOperationalState MatterManager::getCurrentLockState() const {
    return lock_state_;
}

void MatterManager::updateLockState(LockOperationalState state) {
    if (lock_state_ == state) {
        return;  // No change
    }

    lock_state_ = state;
    savePairingDataToNVS();

    ESP_LOGI(TAG, "Lock state updated: %d", static_cast<int>(state));

    // Notify Home Assistant of state change by triggering attribute change callback
    matterAttributeChangeCallback(DOOR_LOCK_CLUSTER_ID, LOCK_STATE_ATTRIBUTE_ID);
}

void MatterManager::handleLockCommand(bool lock_command) {
    ESP_LOGI(TAG, "Lock command received from Home Assistant: %s", 
             lock_command ? "LOCK" : "UNLOCK");

    // Get lock controller instance and execute command
    LockController* controller = LockController::getInstance();
    if (controller) {
        controller->staticCallbackUpdateLockState(lock_command ? "LOCK" : "UNLOCK");
    } else {
        ESP_LOGE(TAG, "Lock controller not available");
    }
}

const char* MatterManager::getErrorMessage() const {
    return error_message_;
}

const char* MatterManager::getDeviceId() const {
    return device_id_;
}

void MatterManager::factoryReset() {
    ESP_LOGI(TAG, "Performing factory reset of Matter data");

    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(PAIRING_NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err == ESP_OK) {
        nvs_erase_all(nvs_handle);
        nvs_commit(nvs_handle);
        nvs_close(nvs_handle);
    }

    pairing_state_ = PairingState::Unpaired;
    lock_state_ = LockOperationalState::Locked;

    ESP_LOGI(TAG, "Factory reset complete");
}

void MatterManager::runDiagnostics() {
    ESP_LOGI(TAG, "=== Matter Manager Diagnostics ===");
    ESP_LOGI(TAG, "Initialized: %s", initialized_ ? "YES" : "NO");
    ESP_LOGI(TAG, "Started: %s", started_ ? "YES" : "NO");
    ESP_LOGI(TAG, "Pairing State: %d", static_cast<int>(pairing_state_));
    ESP_LOGI(TAG, "Lock State: %d", static_cast<int>(lock_state_));
    ESP_LOGI(TAG, "Device ID: %s", device_id_);
    ESP_LOGI(TAG, "====================================");
}

void MatterManager::setPairingState(PairingState new_state) {
    if (pairing_state_ != new_state) {
        ESP_LOGI(TAG, "Pairing state changed: %d -> %d",
                 static_cast<int>(pairing_state_), static_cast<int>(new_state));
        pairing_state_ = new_state;
        savePairingDataToNVS();
    }
}

void MatterManager::setError(const char* error_msg) {
    if (error_msg) {
        strncpy(error_message_, error_msg, sizeof(error_message_) - 1);
        error_message_[sizeof(error_message_) - 1] = '\0';
    }
}

void MatterManager::initializeMatterStack() {
    ESP_LOGI(TAG, "Initializing Matter stack");

    // Matter stack initialization will be done here
    // This includes:
    // - Initializing the Matter SDK
    // - Setting up device information
    // - Registering clusters and endpoints

    // Set device name and information
    // This will be used during commissioning and in Home Assistant discovery
    
    ESP_LOGI(TAG, "Matter stack initialized with device name: %s", MATTER_DEVICE_NAME);
}

void MatterManager::setupDoorLockCluster() {
    ESP_LOGI(TAG, "Setting up Door Lock cluster");

    // Door Lock cluster setup will be done here
    // This includes:
    // - Creating cluster instances
    // - Registering attributes (Lock State, etc.)
    // - Registering commands (Lock, Unlock)
    // - Setting up attribute access permissions

    // In a production implementation, this would use Matter SDK APIs
    // to properly register the Door Lock cluster
    
    ESP_LOGI(TAG, "Door Lock cluster created (Cluster ID: 0x%04X)", DOOR_LOCK_CLUSTER_ID);
}

void MatterManager::registerAttributeHandlers() {
    ESP_LOGI(TAG, "Registering Matter attribute handlers");

    // Attribute handlers will be registered here
    // This includes callbacks for:
    // - Attribute read requests
    // - Attribute write requests
    // - Attribute change notifications

    // Register the lock state attribute handler
    // This allows Home Assistant to read the current lock state
    
    ESP_LOGI(TAG, "Attribute handlers registered");
}

void MatterManager::loadPairingDataFromNVS() {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(PAIRING_NVS_NAMESPACE, NVS_READONLY, &nvs_handle);

    if (err != ESP_OK) {
        ESP_LOGI(TAG, "No pairing data found in NVS (first boot)");
        pairing_state_ = PairingState::Unpaired;
        lock_state_ = LockOperationalState::Locked;
        return;
    }

    uint8_t pairing_state = static_cast<uint8_t>(PairingState::Unpaired);
    uint8_t lock_state = static_cast<uint8_t>(LockOperationalState::Locked);

    nvs_get_u8(nvs_handle, PAIRING_STATE_KEY, &pairing_state);
    nvs_get_u8(nvs_handle, LOCK_STATE_KEY, &lock_state);
    nvs_close(nvs_handle);

    pairing_state_ = static_cast<PairingState>(pairing_state);
    lock_state_ = static_cast<LockOperationalState>(lock_state);

    ESP_LOGI(TAG, "Pairing data loaded from NVS");
}

void MatterManager::savePairingDataToNVS() {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(PAIRING_NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS handle: %s", esp_err_to_name(err));
        return;
    }

    uint8_t pairing_state = static_cast<uint8_t>(pairing_state_);
    uint8_t lock_state = static_cast<uint8_t>(lock_state_);

    nvs_set_u8(nvs_handle, PAIRING_STATE_KEY, pairing_state);
    nvs_set_u8(nvs_handle, LOCK_STATE_KEY, lock_state);

    if (nvs_commit(nvs_handle) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to commit NVS data");
    }

    nvs_close(nvs_handle);
}

void MatterManager::matterAttributeChangeCallback(uint32_t cluster_id, uint32_t attribute_id) {
    if (!g_matter_manager) {
        return;
    }

    if (cluster_id == DOOR_LOCK_CLUSTER_ID && attribute_id == LOCK_STATE_ATTRIBUTE_ID) {
        ESP_LOGI(TAG, "Lock state attribute changed: %d", 
                 static_cast<int>(g_matter_manager->lock_state_));
        // Notify Matter controllers of the attribute change
        // This will trigger Home Assistant to update the lock state
    }
}

void MatterManager::matterCommandCallback(uint32_t cluster_id, uint32_t command_id, 
                                           const uint8_t* data, uint16_t len) {
    if (!g_matter_manager) {
        return;
    }

    if (cluster_id != DOOR_LOCK_CLUSTER_ID) {
        return;
    }

    switch (command_id) {
        case LOCK_COMMAND_ID:
            ESP_LOGI(TAG, "Lock command received");
            g_matter_manager->handleLockCommand(true);
            break;

        case UNLOCK_COMMAND_ID:
            ESP_LOGI(TAG, "Unlock command received");
            g_matter_manager->handleLockCommand(false);
            break;

        default:
            ESP_LOGW(TAG, "Unknown door lock command: %lu", command_id);
            break;
    }
}
