#pragma once

#include "External/limine.h"
#include "RC/CharacterWriter.h"

class LimineTerminal : public CharacterWriter {
public:
    LimineTerminal(u64 terminal_id) : m_terminal_id{terminal_id} {
        // TODO: assert terminal is not null
        // TODO: Assert terminal id exists
    }

    // TODO: Use optional/result for possible errors
    static LimineTerminal default_terminal();

    int write(const char *str);
    int write_char(char c);

private:
    u64 m_terminal_id;

    limine_terminal *current_terminal();
};
