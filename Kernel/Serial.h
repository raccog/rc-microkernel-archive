/** \file
 * Standardized COM port driver.
 */

#pragma once

#include "RC/CharacterWriter.h"

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

class Serial : public CharacterWriter {
    enum ComPort {
        COM1 = 0x3f8,
    };

    Serial(ComPort port) : m_port{port} {}

    ComPort m_port;

    int init();
    bool is_transmit_empty();

public:
    static Serial default_serial();

    int write(const char *str);
    int write_char(char c);
};
