idf_component_register(
    SRCS
        "lock.cpp"
        "lockController.cpp"
        "main.cpp"
        "zigbeeManager.cpp"
    INCLUDE_DIRS "."
    REQUIRES nvs_flash
    PRIV_REQUIRES esp_driver_gpio esp_driver_uart ieee802154
)
