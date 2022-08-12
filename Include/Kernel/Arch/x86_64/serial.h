#ifdef ARCH_x86_64

#pragma once

/**
 * The port address for COM1 serial device.
 */
#define COM1_PORT 0x3f8

/**
 * Initializes a RS-232 serial device.
 *
 * @return Returns a status code. 0 if successful, -1 if failed.
 */
int serial_init();

/**
 * Writes a string to the serial device.
 *
 * @return Returns a status code. 0 if successful, -1 if failed.
 */
int serial_write(const char *str);

/**
 * Writes a single character to the screen-based terminal.
 *
 * @return Returns a status code. 0 if successful, -1 if failed.
 */
int serial_write_char(char c);

/**
 * Check if the serial device is available to transmit a character.
 *
 * @return True if the serial device's transmit buffer is empty.
 */
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

#endif
