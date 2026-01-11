idf_component_register(
    SRCS
        "buzzer.cpp"
        "controller.cpp"
        "lock.cpp"
        "main.cpp"
        "matterManager.cpp"
        "threadManager.cpp"
    INCLUDE_DIRS "."
    REQUIRES esp_wifi esp_event
    PRIV_REQUIRES esp_driver_gpio
)
