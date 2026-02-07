# Setup

You are missing a gitignored file, called `config.hpp`. <br>
Create this file within this directory. The file looks like this:

```
#pragma once

// Relay which controls the lock mechanism
#define RELAY_LOCK_PIN              9

// Factory reset input (active-low). Hold during boot to reset Zigbee storage.
#define FACTORY_RESET_PIN           4

```
