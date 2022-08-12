#ifdef ARCH_x86_64

#pragma once

/**
 * Returns a byte that was recieved from an x86 port.
 *
 * @param port The port to get a byte from
 * @return The byte read from `port`.
 */
u8 inb(u16 port);

/**
 * Outputs a byte to an x86 port.
 *
 * @param port The port to output `value` to.
 * @param value The byte to output to `port`.
 */
void outb(u16 port, u8 value);

#endif
