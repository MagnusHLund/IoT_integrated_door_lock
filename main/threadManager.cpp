#include "threadManager.hpp"
#include "esp_log.h"
#include "string.h"

static const char* TAG = "THREAD_MANAGER";

ThreadManager::ThreadManager()
    : state_(ThreadState::Disabled),
      initialized_(false),
      enabled_(false) {
    memset(network_name_, 0, sizeof(network_name_));
    memset(device_role_, 0, sizeof(device_role_));
}

ThreadManager::~ThreadManager() {
    disable();
}

bool ThreadManager::initialize() {
    if (initialized_) {
        ESP_LOGW(TAG, "Thread Manager already initialized");
        return true;
    }

    initializeThreadStack();
    initialized_ = true;
    setState(ThreadState::Disabled);

    ESP_LOGI(TAG, "Thread Manager initialized successfully");
    return true;
}

bool ThreadManager::enable() {
    if (!initialized_) {
        ESP_LOGE(TAG, "Thread Manager not initialized");
        return false;
    }

    if (enabled_) {
        ESP_LOGW(TAG, "Thread already enabled");
        return true;
    }

    enabled_ = true;
    setState(ThreadState::Searching);

    ESP_LOGI(TAG, "Thread enabled - searching for border router");
    return true;
}

bool ThreadManager::disable() {
    if (!enabled_) {
        return true;
    }

    enabled_ = false;
    setState(ThreadState::Disabled);

    ESP_LOGI(TAG, "Thread disabled");
    return true;
}

ThreadManager::ThreadState ThreadManager::getState() const {
    return state_;
}

bool ThreadManager::isConnected() const {
    return (state_ == ThreadState::ChildSecured ||
            state_ == ThreadState::RouterSecured ||
            state_ == ThreadState::Leader);
}

const char* ThreadManager::getNetworkName() const {
    return network_name_;
}

const char* ThreadManager::getDeviceRole() const {
    return device_role_;
}

void ThreadManager::runDiagnostics() {
    ESP_LOGI(TAG, "=== Thread Network Diagnostics ===");
    ESP_LOGI(TAG, "State: %d", static_cast<int>(state_));
    ESP_LOGI(TAG, "Connected: %s", isConnected() ? "YES" : "NO");
    ESP_LOGI(TAG, "Network Name: %s", network_name_[0] ? network_name_ : "N/A");
    ESP_LOGI(TAG, "Device Role: %s", device_role_[0] ? device_role_ : "N/A");
    ESP_LOGI(TAG, "===================================");
}

void ThreadManager::setState(ThreadState new_state) {
    if (state_ != new_state) {
        ESP_LOGI(TAG, "Thread state changed: %d -> %d", 
                 static_cast<int>(state_), static_cast<int>(new_state));
        state_ = new_state;
    }
}

void ThreadManager::initializeThreadStack() {
    // Thread stack initialization will be done here
    // This typically involves:
    // - Initializing OpenThread
    // - Setting up border router provisioning
    // - Registering state change callbacks
    
    ESP_LOGI(TAG, "Thread stack initialized");
}

void ThreadManager::handleThreadStateChange() {
    // Called when Thread network state changes
    // Updates device role and network information
    logThreadInfo();
}

void ThreadManager::logThreadInfo() {
    ESP_LOGD(TAG, "Thread network information updated");
}