/** \file
 * Standardized COM port driver.
 */

#pragma once

/**
 * Returns a byte input from a port.
 *
 * @param port The port to input a byte from
 */
extern "C" u8 inb(u16 port);

/**
 * Outputs a byte to a port number.
 *
 * @param port The port to output to.
 * @param value The value to output to the port.
 */
extern "C" void outb(u16 port, u8 value);

/**
 * Initializes the COM port.
 */
int kinit_serial();

/**
 * Sends a character to the COM port.
 */
void kputc_serial(char c);
