#pragma once

#include "character_device.h"

/**
 * Print a formatted string to standard output.
 *
 * @param format The format string. Should be C-style formatting.
 * @param ... Arguments to be formatted into the format string.
 * @return Returns status code. 0 if successful, -1 if failed.
 */
int printk(const char *format, ...) __attribute__((format(printf, 1, 2)));

/**
 * Print a formatted string to debug output.
 *
 * @param format The format string. Should be C-style formatting.
 * @param ... Arguments to be formatted into the format string.
 * @return Returns status code. 0 if successful, -1 if failed.
 */
int debugk(const char *format, ...) __attribute__((format(printf, 1, 2)));
