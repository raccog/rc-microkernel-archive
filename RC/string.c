#include "string.h"

void *memcpy(void *dest, const void *src, size_t count) {
    u8 *dest_bytes = (u8 *)dest;
    const u8 *src_bytes = (const u8 *)src;
    for (size_t i = 0; i < count; ++i) {
        dest_bytes[i] = src_bytes[i];
    }
    return dest;
}

size_t strlen(const char *str) {
    size_t len = 0;
    for (; *str != '\0'; ++str) {
        ++len;
    }
    return len;
}
