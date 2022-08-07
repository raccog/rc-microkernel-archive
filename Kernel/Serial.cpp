/** \file
 * COM port driver implementation.
 */

#include "Serial.h"

constexpr u16 PORT_COM1 = 0x3f8;

// taken from https://wiki.osdev.org/Serial_Ports
int kinit_serial() {
    outb(PORT_COM1 + 1, 0x00); // disable serial interrupts
    outb(PORT_COM1 + 3, 0x80); // disable DLAB (set baud rate divisor)
    outb(PORT_COM1 + 0, 0x03); // set divisor to 3 (lo byte) 38400 baud
    outb(PORT_COM1 + 1, 0x00); //                  (hi byte)
    outb(PORT_COM1 + 3, 0x03); // 8 bits, no parity, 1 stop bit
    outb(PORT_COM1 + 2, 0xc7); // enable FIFO, clear them, with 14-byte
                               // threshold
    outb(PORT_COM1 + 4, 0x0b); // IRQs enabled, RTS/DSR set
    outb(PORT_COM1 + 4, 0x1e); // set in loopback mode, test serial chip
    outb(PORT_COM1 + 0, 0xae); // test serial chip (send byte 0xae and check
                               // if serial returns same byte)

    if (inb(PORT_COM1) != 0xae) {
        return -1;
    }

    // set normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    outb(PORT_COM1 + 4, 0xf);

    return 0;
}

bool is_transmit_empty() { return inb(PORT_COM1 + 5) & 0x20; }

void kputc_serial(char c) {
    while (is_transmit_empty() == 0)
        ;

    outb(PORT_COM1, c);
}
