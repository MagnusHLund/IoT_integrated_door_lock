idf_component_register(
    SRCS
        "lock.cpp"
        "lockController.cpp"
        "main.cpp"
        "wiFiManager.cpp"
        "mqttManager.cpp"
        "pairing.cpp"
    INCLUDE_DIRS "."
    REQUIRES esp_event esp_timer esp_wifi nvs_flash esp_netif mqtt
    PRIV_REQUIRES esp_driver_gpio
)
