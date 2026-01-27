# SmartCrib ESP-IDF CMakeLists

idf_component_register(SRCS "main.c" "mqtt_config.c" "wifi_config.c" "drv_mpu.c" "drv_loadcell.c"
                    INCLUDE_DIRS "../include"
                    EMBED_TXTFILES "../certs/root_ca.pem")