idf_component_register(
    SRCS
        "bleCommissioning.cpp"
        "button.cpp"
        "buzzer.cpp"
        "lock.cpp"
        "lockController.cpp"
        "main.cpp"
        "matterManager.cpp"
        "reset.cpp"
        "threadManager.cpp"
    INCLUDE_DIRS "."
    REQUIRES esp_event esp_timer nvs_flash
    PRIV_REQUIRES esp_driver_gpio
)
