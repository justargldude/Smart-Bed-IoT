# SmartCrib ESP-IDF CMakeLists

idf_component_register(
    SRCS "main.c" "mqtt_config.c" "wifi_config.c" "drv_loadcell.c" "drv_mpu.c"
    INCLUDE_DIRS "../include"
    EMBED_TXTFILES "../certs/root_ca.pem"
)