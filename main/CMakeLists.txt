idf_component_register(SRCS "lock.cpp" "buzzer.cpp" "main.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES esp_wifi esp_event
                    PRIV_REQUIRES esp_driver_gpio
)
