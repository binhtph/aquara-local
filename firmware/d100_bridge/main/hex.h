#pragma once
#include <stddef.h>
#include <stdint.h>

// Decode `n_hex` chars from `hex` into `out` (n_hex/2 bytes). Returns 0 on success,
// -1 on bad char / odd length. Lowercase or uppercase both accepted.
int hex_decode(const char *hex, size_t n_hex, uint8_t *out);

// Encode `n` bytes into `out` (lowercase, no separator). Returns number of chars (= 2*n).
// `out` must have space for 2*n + 1 (NUL terminator). Caller writes NUL.
size_t hex_encode(const uint8_t *in, size_t n, char *out);

// Reverse a MAC byte order in place (BLE peripheral order ↔ Aqara cloud order).
void mac_reverse(uint8_t mac[6]);
