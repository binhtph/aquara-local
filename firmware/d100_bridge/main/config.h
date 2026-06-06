#pragma once
// Compile-time defaults. Override at runtime via NVS namespace "bridge_cfg"
// (see README), so the same binary works on any board / network without rebuild.

#ifndef BRIDGE_WIFI_SSID
#define BRIDGE_WIFI_SSID    "SmartHome"
#endif
#ifndef BRIDGE_WIFI_PASS
#define BRIDGE_WIFI_PASS    ""        // empty → read from NVS at runtime
#endif

#ifndef BRIDGE_TCP_PORT
#define BRIDGE_TCP_PORT     8888
#endif

// Static IP for predictability (matches the IP the existing proxy used).
// Leave empty string to use DHCP.
#ifndef BRIDGE_STATIC_IP
#define BRIDGE_STATIC_IP    "192.168.2.162"
#endif
#ifndef BRIDGE_GATEWAY
#define BRIDGE_GATEWAY      "192.168.2.1"
#endif
#ifndef BRIDGE_NETMASK
#define BRIDGE_NETMASK      "255.255.255.0"
#endif

// Max line length in the TCP protocol (one BLE write data + header).
#define BRIDGE_LINE_MAX     4096

// One BLE peripheral at a time.
#define BRIDGE_MAX_CHARS    32
