#pragma once

/**
 * Copies `count` bytes from `src` to `dest`.
 *
 * @param dest The destination address.
 * @param src The source address.
 * @return `dest`
 */
void *memcpy(void *dest, const void *src, size_t count);

/**
 * Returns the length of `str` in bytes.
 *
 * @param str Returns the length of this string.
 * @return The length of `str` in bytes.
 */
size_t strlen(const char *str);
