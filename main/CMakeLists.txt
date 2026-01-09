idf_component_register(SRCS "lock.c" "buzzer.c" "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_wifi esp_event)
