/** \file
 * Standardized COM port driver.
 */

#pragma once

#include "RC/CharacterWriter.h"

class Serial : public CharacterWriter {
    enum ComPort {
        COM1 = 0x3f8,
    };

    ComPort m_port;
    bool m_initialized;

    int init();
    bool is_transmit_empty();

public:
    Serial() : m_port{COM1}, m_initialized{false} {}
    Serial(ComPort port) : m_port{port}, m_initialized{false} {}

    static Serial &init_default();

    int write(const char *str);
    int write_char(char c);
};

/* Temporary function
 *
 * Print signed integer to serial output.
 */
void kserial_int(i64 value);

/* Temporary function
 *
 * Print unsigned integer to serial output.
 */
void kserial_uint(u64 value);

/* Temporary function
 *
 * Print hex integer to serial output.
 */
void kserial_hex(u64 value);
