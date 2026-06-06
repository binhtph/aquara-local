#include "wifi.h"
#include "config.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "nvs.h"
#include <string.h>

#define TAG "wifi"
static EventGroupHandle_t s_evt;
#define BIT_GOT_IP BIT0

static void on_event(void *arg, esp_event_base_t base, int32_t id, void *data) {
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "disconnected, retrying");
        esp_wifi_connect();
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *e = (ip_event_got_ip_t *)data;
        ESP_LOGI(TAG, "got ip " IPSTR, IP2STR(&e->ip_info.ip));
        xEventGroupSetBits(s_evt, BIT_GOT_IP);
    }
}

static void read_cfg(char *ssid, size_t ssid_n, char *pass, size_t pass_n) {
    // Compile-time defaults first.
    strncpy(ssid, BRIDGE_WIFI_SSID, ssid_n - 1);
    strncpy(pass, BRIDGE_WIFI_PASS, pass_n - 1);
    nvs_handle_t h;
    if (nvs_open("bridge_cfg", NVS_READONLY, &h) != ESP_OK) return;
    size_t n = ssid_n;
    nvs_get_str(h, "wifi_ssid", ssid, &n);
    n = pass_n;
    nvs_get_str(h, "wifi_pass", pass, &n);
    nvs_close(h);
}

static void apply_static_ip(esp_netif_t *sta) {
#ifdef BRIDGE_STATIC_IP
    if (BRIDGE_STATIC_IP[0] == 0) return;
    esp_netif_dhcpc_stop(sta);
    esp_netif_ip_info_t ip = {0};
    ip.ip.addr      = esp_ip4addr_aton(BRIDGE_STATIC_IP);
    ip.gw.addr      = esp_ip4addr_aton(BRIDGE_GATEWAY);
    ip.netmask.addr = esp_ip4addr_aton(BRIDGE_NETMASK);
    esp_netif_set_ip_info(sta, &ip);
    esp_netif_dns_info_t dns = {.ip.u_addr.ip4.addr = esp_ip4addr_aton("8.8.8.8"),
                                .ip.type = ESP_IPADDR_TYPE_V4};
    esp_netif_set_dns_info(sta, ESP_NETIF_DNS_MAIN, &dns);
#endif
}

void wifi_start_blocking(void) {
    s_evt = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta = esp_netif_create_default_wifi_sta();
    apply_static_ip(sta);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, on_event, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, on_event, NULL));

    char ssid[33] = {0}, pass[65] = {0};
    read_cfg(ssid, sizeof ssid, pass, sizeof pass);
    if (ssid[0] == 0) {
        ESP_LOGE(TAG, "no SSID — set via NVS namespace bridge_cfg key wifi_ssid");
        // Allow user to provision via console / future hotspot; just spin.
        while (1) vTaskDelay(pdMS_TO_TICKS(60000));
    }
    wifi_config_t wc = {0};
    strncpy((char *)wc.sta.ssid, ssid, sizeof wc.sta.ssid - 1);
    strncpy((char *)wc.sta.password, pass, sizeof wc.sta.password - 1);
    wc.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wc));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "connecting to %s", ssid);
    xEventGroupWaitBits(s_evt, BIT_GOT_IP, pdFALSE, pdTRUE, portMAX_DELAY);
}
