#pragma once

#include <cstdint>
#include <stdbool.h>

class ThreadManager;

/**
 * @brief Matter Protocol Manager for Home Assistant Integration
 * 
 * Manages the Matter protocol stack and handles communication with
 * Home Assistant and other Matter controllers. Implements the DoorLock
 * cluster for lock/unlock operations with proper state management.
 * 
 * Features:
 * - Matter protocol initialization
 * - Door Lock cluster implementation
 * - Home Assistant commissioning integration
 * - Attribute change callbacks
 * - Persistent pairing data storage
 */
class MatterManager {
public:
    enum class PairingState {
        Unpaired,
        Commissioning,
        Paired,
        Error
    };

    enum class LockOperationalState {
        Locked = 0,
        Unlocked = 1,
        UnknownOrError = 3
    };

    MatterManager();
    ~MatterManager();

    /**
     * @brief Initialize Matter protocol stack
     * Should be called during device startup
     * @param threadManager Reference to initialized ThreadManager
     * @return true if initialization was successful
     */
    bool initialize(ThreadManager& threadManager);

    /**
     * @brief Start Matter protocol and BLE advertisement
     * @return true if startup was successful
     */
    bool start();

    /**
     * @brief Get current pairing state
     * @return Current PairingState
     */
    PairingState getPairingState() const;

    /**
     * @brief Check if device is paired with a Matter controller
     * @return true if device is commissioned
     */
    bool isPaired() const;

    /**
     * @brief Get current lock operational state
     * @return Current LockOperationalState
     */
    LockOperationalState getCurrentLockState() const;

    /**
     * @brief Update lock operational state
     * Called when lock is physically locked/unlocked
     * Triggers attribute change notification to Home Assistant
     * @param state New lock state
     */
    void updateLockState(LockOperationalState state);

    /**
     * @brief Handle lock command from Home Assistant
     * Called by Matter command handler
     * @param lock_command true to lock, false to unlock
     */
    void handleLockCommand(bool lock_command);

    /**
     * @brief Get last error message
     * @return Error message string, or empty string if no error
     */
    const char* getErrorMessage() const;

    /**
     * @brief Get device unique identifier
     * @return Device ID string
     */
    const char* getDeviceId() const;

    /**
     * @brief Perform factory reset of Matter data
     * Clears all pairing and fabric information
     */
    void factoryReset();

    /**
     * @brief Run Matter diagnostics
     * Logs device information and cluster status
     */
    void runDiagnostics();

private:
    PairingState pairing_state_;
    LockOperationalState lock_state_;
    bool initialized_;
    bool started_;
    char error_message_[256];
    char device_id_[64];
    
    ThreadManager* thread_manager_;

    // Private methods
    void setPairingState(PairingState new_state);
    void setError(const char* error_msg);
    void initializeMatterStack();
    void setupDoorLockCluster();
    void registerAttributeHandlers();
    void loadPairingDataFromNVS();
    void savePairingDataToNVS();

    // Matter callbacks (static, forward to member methods)
    static void matterAttributeChangeCallback(uint32_t cluster_id, uint32_t attribute_id);
    static void matterCommandCallback(uint32_t cluster_id, uint32_t command_id, const uint8_t* data, uint16_t len);
