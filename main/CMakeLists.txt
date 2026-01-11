idf_component_register(
    SRCS
        "button.cpp"
        "buzzer.cpp"
        "lock.cpp"
        "lockController.cpp"
        "main.cpp"
        "matterManager.cpp"
        "reset.cpp"
        "threadManager.cpp"
    INCLUDE_DIRS "."
    REQUIRES esp_wifi esp_event
    PRIV_REQUIRES esp_driver_gpio
)
