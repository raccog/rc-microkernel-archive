/** \file
 * Standardized COM port driver.
 */

#pragma once

#define COM1_PORT 0x3f8

int serial_init();
int serial_write(const char *str);
int serial_write_char(char c);
bool serial_is_transmit_empty();

/* Temporary function
 *
 * Print signed integer to serial output.
 */
void serial_write_int(i64 value);

/* Temporary function
 *
 * Print unsigned integer to serial output.
 */
void serial_write_uint(u64 value);

/* Temporary function
 *
 * Print hex integer to serial output.
 */
void serial_write_hex(u64 value);
