idf_component_register(SRCS "blink_example_main.c"
                    INCLUDE_DIRS "."
                REQUIRES driver esp_timer esp_driver_uart esp_driver_gpio log)
