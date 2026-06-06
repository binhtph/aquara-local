#pragma once

// Block until WiFi STA is connected and has an IP. Reads SSID/PASS from NVS first
// (namespace "bridge_cfg", keys "wifi_ssid"/"wifi_pass"), falls back to compile-time
// BRIDGE_WIFI_SSID/PASS in config.h.
void wifi_start_blocking(void);
