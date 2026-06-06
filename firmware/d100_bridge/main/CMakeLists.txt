idf_component_register(
    SRCS
        "main.c"
        "wifi.c"
        "tcp_server.c"
        "ble_central.c"
        "proto.c"
        "hex.c"
    INCLUDE_DIRS "."
    REQUIRES
        nvs_flash
        esp_wifi
        esp_netif
        esp_event
        lwip
        bt
)
