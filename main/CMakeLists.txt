idf_component_register(SRCS "buzzer.cpp" "buzzer.hpp" "controller.cpp" "controller.hpp" "lock.cpp" "lock.hpp" "main.cpp" "matterController.cpp" "matterController.hpp"
                    INCLUDE_DIRS "."
                    REQUIRES esp_wifi esp_event
                    PRIV_REQUIRES esp_driver_gpio
)
