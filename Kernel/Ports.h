#pragma once

/**
 * Returns a byte input from a port.
 *
 * @param port The port to input a byte from
 * @return The byte read from `port`.
 */
extern "C" u8 inb(u16 port);

/**
 * Outputs a byte to a port number.
 *
 * @param port The port to output to.
 * @param value The value to output to the port.
 */
extern "C" void outb(u16 port, u8 value);
