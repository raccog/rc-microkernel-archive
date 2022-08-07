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

    int init();
    bool is_transmit_empty();

public:
    Serial() : m_port{COM1} {}
    Serial(ComPort port) : m_port{port} {}

    static Serial init_default();

    int write(const char *str);
    int write_char(char c);
};
