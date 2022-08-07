/** \file
 * COM port driver implementation.
 */

#include "Serial.h"

#include "Panic.h"
#include "Ports.h"

/*
 * COM1 port serial device.
 */
static Serial SERIAL_COM1;

int Serial::write(const char *str) {
    if (!m_initialized)
        return -1;

    while (*str != '\0') {
        write_char(*str);
        ++str;
    }
    return 0;
}

int Serial::write_char(char c) {
    if (!m_initialized)
        return -1;

    while (is_transmit_empty() == 0)
        ;

    outb(m_port, c);

    return 0;
}

bool Serial::is_transmit_empty() { return inb(m_port + 5) & 0x20; }

Serial &Serial::init_default() {
    if (SERIAL_COM1.m_initialized)
        return SERIAL_COM1;

    if (SERIAL_COM1.init() != 0)
        panic();

    return SERIAL_COM1;
}

// taken from https://wiki.osdev.org/Serial_Ports
int Serial::init() {
    m_initialized = false;

    outb(m_port + 1, 0x00); // disable serial interrupts
    outb(m_port + 3, 0x80); // disable DLAB (set baud rate divisor)
    outb(m_port + 0, 0x03); // set divisor to 3 (lo byte) 38400 baud
    outb(m_port + 1, 0x00); //                  (hi byte)
    outb(m_port + 3, 0x03); // 8 bits, no parity, 1 stop bit
    outb(m_port + 2, 0xc7); // enable FIFO, clear them, with 14-byte
                            // threshold
    outb(m_port + 4, 0x0b); // IRQs enabled, RTS/DSR set
    outb(m_port + 4, 0x1e); // set in loopback mode, test serial chip
    outb(m_port + 0, 0xae); // test serial chip (send byte 0xae and check
                            // if serial returns same byte)

    if (inb(m_port) != 0xae) {
        return -1;
    }

    // set normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    outb(m_port + 4, 0xf);

    m_initialized = true;

    return 0;
}

void kserial_int(i64 value) {

    if (value == 0) {
        SERIAL_COM1.write_char('0');
    } else {
        i64 comparator = -1000000000000000000;

        // print negative sign if needed
        if (value < 0) {
            SERIAL_COM1.write_char('-');
        }

        // convert positive to negative for comparisons
        if (value > 0) {
            value *= -1;
        }

        // move comparator to first digit of value
        while (comparator < value) {
            comparator /= 10;
        }

        // print digits
        while (comparator < 0) {
            char digit = '0';
            if (comparator != -1) {
                digit += (char)((value / comparator) % 10);
            } else {
                digit += (char)((value % 10) * -1);
            }
            SERIAL_COM1.write_char(digit);
            comparator /= 10;
        }
    }

    SERIAL_COM1.write_char('\n');
}

void kserial_uint(u64 value) {}

void kserial_hex(u64 value) {}
