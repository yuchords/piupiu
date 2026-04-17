idf_component_register(SRCS "main.cpp"
                    INCLUDE_DIRS
                    "."
                    REQUIRES
                    driver
                    nvs_flash
                    esp_timer
                    esp_lcd
                    esp_lcd_st7796
                    lvgl
                    XPowersLib
)
