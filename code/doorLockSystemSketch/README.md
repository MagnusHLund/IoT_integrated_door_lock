# Setup

You are missing a gitignored file, called `config.h`. <br>
Create this file within this directory. The file looks like this:

```
#pragma once

#define RELAY_LOCK_PIN 9

#define KEYPAD_ROW_1_PIN 13
#define KEYPAD_ROW_2_PIN 12
#define KEYPAD_ROW_3_PIN 11
#define KEYPAD_ROW_4_PIN 10
#define KEYPAD_COL_1_PIN 8
#define KEYPAD_COL_2_PIN 1
#define KEYPAD_COL_3_PIN 0
#define KEYPAD_COL_4_PIN 7

#define BUZZER_PIN 2

#define WIFI_SSID ""
#define WIFI_PASSWORD ""
#define WIFI_STATIC_IP "x.x.x.x"
#define WIFI_SUBNET_MASK "x.x.x.x"
#define WIFI_GATEWAY "x.x.x.x"
#define WIFI_DNS_SERVER "x.x.x.x"

#define MQTT_HOSTNAME "x.x.x.x"
#define MQTT_PORT 1883
#define MQTT_USERNAME ""
#define MQTT_PASSWORD ""
```
