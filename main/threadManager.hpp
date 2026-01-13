#pragma once

#include <cstdint>
#include <stdbool.h>

/**
 * @brief Thread Network Manager for Matter device
 * 
 * Handles Thread border router connectivity and network provisioning.
 * Thread is required for reliable Matter communication with Home Assistant.
 * 
 * Features:
 * - Thread network initialization
 * - Border router discovery and connection
 * - Automatic network join after commissioning
 * - Network state monitoring
 * - Thread dataset management
 */
class ThreadManager {
public:
    enum class ThreadState {
        Disabled,
        Searching,
        Attaching,
        Attached,
        Leader,
        ChildSecured,
        RouterSecured,
        Error
    };

    ThreadManager();
    ~ThreadManager();

    /**
     * @brief Initialize Thread network subsystem
     * @return true if initialization was successful
     */
    bool initialize();

    /**
     * @brief Enable Thread and start joining network
     * Should be called after Matter commissioning is complete
     * @return true if Thread enable was successful
     */
    bool enable();

    /**
     * @brief Disable Thread network
     * @return true if Thread was disabled successfully
     */
    bool disable();

    /**
     * @brief Get current Thread network state
     * @return Current ThreadState
     */
    ThreadState getState() const;

    /**
     * @brief Check if device is connected to Thread network
     * @return true if device has a valid Thread connection
     */
    bool isConnected() const;

    /**
     * @brief Get Thread network name (Extended PAN ID)
     * @return Network name string, or empty string if not connected
     */
    const char* getNetworkName() const;

    /**
     * @brief Get device role in Thread network
     * @return Role string (child, router, leader)
     */
    const char* getDeviceRole() const;

    /**
     * @brief Perform Thread network diagnostics
     * Logs current network parameters and device role
     */
    void runDiagnostics();

private:
    ThreadState state_;
    bool initialized_;
    bool enabled_;
    char network_name_[64];
    char device_role_[32];

    void setState(ThreadState new_state);
    void initializeThreadStack();
    void handleThreadStateChange();
    void logThreadInfo();
};