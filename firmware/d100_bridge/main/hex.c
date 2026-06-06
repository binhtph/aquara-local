#include "hex.h"

static int nibble(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

int hex_decode(const char *hex, size_t n_hex, uint8_t *out) {
    if (n_hex & 1) return -1;
    for (size_t i = 0; i < n_hex; i += 2) {
        int hi = nibble(hex[i]), lo = nibble(hex[i + 1]);
        if (hi < 0 || lo < 0) return -1;
        out[i / 2] = (uint8_t)((hi << 4) | lo);
    }
    return 0;
}

size_t hex_encode(const uint8_t *in, size_t n, char *out) {
    static const char H[] = "0123456789abcdef";
    for (size_t i = 0; i < n; i++) {
        out[2 * i]     = H[in[i] >> 4];
        out[2 * i + 1] = H[in[i] & 0xf];
    }
    return 2 * n;
}

void mac_reverse(uint8_t mac[6]) {
    for (int i = 0; i < 3; i++) {
        uint8_t t = mac[i];
        mac[i] = mac[5 - i];
        mac[5 - i] = t;
    }
}
