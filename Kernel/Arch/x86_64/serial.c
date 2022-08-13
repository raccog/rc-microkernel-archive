#ifdef ARCH_x86_64

#include "Arch/x86_64/serial.h"

#include "Arch/x86_64/ports.h"
#include "character_device.h"

int serial_init() {
    // Taken from https://wiki.osdev.org/Serial_Ports#Initialization
    outb(COM1_PORT + 1, 0x00); // disable serial interrupts
    outb(COM1_PORT + 3, 0x80); // disable DLAB (set baud rate divisor)
    outb(COM1_PORT + 0, 0x03); // set divisor to 3 (lo byte) 38400 baud
    outb(COM1_PORT + 1, 0x00); //                  (hi byte)
    outb(COM1_PORT + 3, 0x03); // 8 bits, no parity, 1 stop bit
    outb(COM1_PORT + 2, 0xc7); // enable FIFO, clear them, with 14-byte
                               // threshold
    outb(COM1_PORT + 4, 0x0b); // IRQs enabled, RTS/DSR set
    outb(COM1_PORT + 4, 0x1e); // set in loopback mode, test serial chip

    // return error if serial is still not initialized
    outb(COM1_PORT + 0, '~'); // test serial chip (send byte 0xae and check
                              // if serial returns same byte)

    if (inb(COM1_PORT) != '~') {
        return -1;
    }

    // set normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    outb(COM1_PORT + 4, 0xf);

    stddebug = (struct kernel_chardev){.write = &serial_write,
                                       .write_char = &serial_write_char};

    return 0;
}

int serial_write(const char *str) {
    int count = 0;
    while (*str != '\0') {
        serial_write_char(*str++);
        ++count;
    }
    return 0;
}

int serial_write_char(char c) {
    while (serial_is_transmit_empty() == 0)
        ;

    outb(COM1_PORT, c);

    return 1;
}

bool serial_is_transmit_empty() { return inb(COM1_PORT + 5) & 0x20; }

void serial_write_int(i64 value) {

    if (value == 0) {
        serial_write_char('0');
    } else {
        // smallest signed integer that is a power of 10
        i64 comparator = -1000000000000000000;

        // print negative sign if needed
        if (value < 0) {
            serial_write_char('-');
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
            serial_write_char(digit);
            comparator /= 10;
        }
    }

    serial_write_char('\n');
}

void serial_write_uint(u64 value) {
    if (value == 0) {
        serial_write_char('0');
    } else {
        // largest unsigned integer that is a power of 10
        u64 comparator = 10000000000000000000ul;

        // move comparator to first digit of value
        while (comparator > value) {
            comparator /= 10;
        }

        // print digits
        while (comparator > 0) {
            char digit = '0' + (char)((value / comparator) % 10);
            serial_write_char(digit);
            comparator /= 10;
        }
    }

    serial_write_char('\n');
}

void serial_write_hex(u64 value) {
    serial_write("0x");

    if (value == 0) {
        serial_write_char('0');
    } else {
        u64 bit_size = 64;

        // move comparator to first digit of value
        while (((value >> (bit_size - 4)) & 0xf) == 0) {
            bit_size -= 4;
        }

        // print digits
        while (bit_size >= 4) {
            char digit = (char)((value >> (bit_size - 4)) & 0xf);
            bit_size -= 4;
            if (digit > 9) {
                digit += 'a' - 0xa;
            } else {
                digit += '0';
            }
            serial_write_char(digit);
        }
    }

    serial_write_char('\n');
}

#endif
