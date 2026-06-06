#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Callback invoked on every notification from a SUBscribed characteristic.
// Args owned by caller; do NOT keep pointers past the call.
typedef void (*ble_notify_cb_t)(uint16_t val_handle, const uint8_t *data, size_t len, void *user);

// Callback invoked when the active peer link drops (any reason).
typedef void (*ble_disc_cb_t)(uint8_t reason, void *user);

// Char descriptor exposed to the TCP layer.
typedef struct {
    uint16_t svc_uuid16;     // 0 if 128-bit (not exposed yet — D100 uses 16-bit only)
    uint16_t chr_uuid16;
    uint16_t val_handle;
    uint8_t  properties;     // BLE_GATT_CHR_PROP_*
    uint16_t cccd_handle;    // 0xFFFF if none
} ble_char_t;

// Init NimBLE host + controller. Must be called once at boot.
void ble_init(void);

// Install user callbacks for async events (notifications, disconnect).
void ble_set_callbacks(ble_notify_cb_t notify, ble_disc_cb_t disc, void *user);

// Scan synchronously for `mac` (6-byte little-endian = BLE wire order). Returns
// >=0 (rssi) and sets *addr_type on success, -1 on timeout.
int ble_scan_for(const uint8_t mac[6], uint32_t timeout_ms, uint8_t *addr_type);

// GAP+GATT connect. Returns 0 + sets *mtu on success, negative NimBLE rc on failure.
int ble_connect(const uint8_t mac[6], uint8_t addr_type, uint16_t *mtu);

// Walk services + characteristics into the internal table.
// On success returns number of characteristics filled into out[] (capped at out_max).
int ble_discover(ble_char_t *out, size_t out_max);

// Subscribe / unsubscribe for notifications on the characteristic at val_handle.
// Requires DISCOVER to have run (uses cached CCCD handle).
int ble_subscribe(uint16_t val_handle, bool enable);

// Write with or without response. Returns 0 on success, negative rc on failure.
int ble_write(uint16_t val_handle, const uint8_t *data, size_t len, bool with_response);

// Read characteristic — blocks until reply or timeout. *out_len updated.
int ble_read(uint16_t val_handle, uint8_t *out, size_t *out_len, size_t out_cap);

// Force-disconnect the active peer (if any). No-op if not connected.
void ble_disconnect(void);

bool ble_is_connected(void);
