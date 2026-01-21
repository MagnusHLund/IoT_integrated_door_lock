idf_component_register(
    SRCS
        "button.cpp"
        "buzzer.cpp"
        "lock.cpp"
        "lockController.cpp"
        "main.cpp"
        "reset.cpp"
        "wiFiManager.cpp"
        "mqttManager.cpp"
    INCLUDE_DIRS "."
    REQUIRES esp_event esp_timer esp_wifi nvs_flash esp_netif mqtt
    PRIV_REQUIRES esp_driver_gpio
)
