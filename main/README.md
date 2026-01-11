# Setup

You are missing a gitignored file, called `config.hpp`. <br>
Create this file within this directory. The file looks like this:

```
#pragma once

// Matter configuration
#define MATTER_DEVICE_NAME          "Smart Door Lock"
#define MATTER_VENDOR_ID            0xFFF1  // Test vendor ID
#define MATTER_PRODUCT_ID           0x8001  // Door lock product ID
#define MATTER_DISCRIMINATOR        3840    // Default discriminator
#define MATTER_PASSCODE             20202021 // Default setup passcode

// Thread network configuration
// These will be provisioned during commissioning
#define THREAD_NETWORK_NAME         "MatterThread"

// Buzzer
#define BUZZER_PIN                  2

// Relay which controls the lock mechanism
#define LOCK_RELAY_PIN              9

// Button for factory reset
#define RESET_BUTTON_PIN            5
```
