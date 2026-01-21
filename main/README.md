# Setup

You are missing a gitignored file, called `config.hpp`. <br>
Create this file within this directory. The file looks like this:

```
#pragma once

// WiFi configuration
#define WIFI_SSID                   "ssid"
#define WIFI_PASSWORD               "password"
#define WIFI_STATIC_IP              "x.x.x.x"
#define WIFI_SUBNET_MASK            "x.x.x.x"
#define WIFI_GATEWAY                "x.x.x.x"
#define WIFI_DNS_SERVER             "x.x.x.x"

// MQTT configuration
#define MQTT_HOSTNAME               "x.x.x.x"
#define MQTT_PORT                   1883
#define MQTT_USERNAME               "username"  // Set if broker requires authentication
#define MQTT_PASSWORD               "password"  // Set if broker requires authentication

// Relay which controls the lock mechanism
#define RELAY_LOCK_PIN              9
```
