#pragma once

#include <cstdint>
#include <cstddef>

/**
 * @brief BLE Commissioning Manager for Matter device
 * 
 * Handles BLE-based commissioning of the Matter device, allowing
 * Home Assistant and other controllers to provision the device
 * with Thread network credentials and Matter operational certificates.
 * 
 * Features:
 * - BLE advertisement with Matter-specific UUIDs
 * - Secure commissioning flow over BLE
 * - Matter setup code exchange
 * - Thread network provisioning
 * - NVS storage of commissioning data
 */
class BLECommissioning {
public:
    enum class CommissioningState {
        Idle,
        Advertising,
        Connected,
        CommissioningInProgress,
        CommissioningComplete,
        Error
    };

    BLECommissioning();
    ~BLECommissioning();

    /**
     * @brief Initialize BLE commissioning subsystem
     * @return true if initialization was successful
     */
    bool initialize();

    /**
     * @brief Start BLE advertisement for commissioning
     * Advertises with Matter-specific UUIDs and setup code
     * @return true if advertisement started successfully
     */
    bool startAdvertising();

    /**
     * @brief Stop BLE advertisement
     * @return true if advertisement stopped successfully
     */
    bool stopAdvertising();

    /**
     * @brief Get current commissioning state
     * @return Current CommissioningState
     */
    CommissioningState getState() const;

    /**
     * @brief Check if device is currently commissioned
     * @return true if device has been commissioned
     */
    bool isCommissioned() const;

    /**
     * @brief Perform factory reset of commissioning data
     * Clears NVS storage and resets BLE state
     */
    void factoryReset();

    /**
     * @brief Get BLE commissioning error message
     * @return Error message string, or empty string if no error
     */
    const char* getErrorMessage() const;

private:
    CommissioningState state_;
    bool initialized_;
    bool commissioned_;
    char error_message_[256];

    // BLE callbacks
    static void bleAdvertisingCallback(void* arg);
    static void bleConnectionCallback(uint16_t conn_handle);
    static void bleDisconnectionCallback(uint16_t conn_handle);
    static void bleCharacteristicWriteCallback(uint16_t conn_handle, uint16_t attr_handle, 
                                               const uint8_t* data, uint16_t len);

    void initializeBLEStack();
    void createCommissioningCharacteristics();
    void handleCommissioningData(const uint8_t* data, uint16_t len);
    void loadCommissioningDataFromNVS();
    void saveCommissioningDataToNVS();
    void setState(CommissioningState new_state);
    void setError(const char* error_msg);
};
