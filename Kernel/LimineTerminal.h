#pragma once

#include "External/limine.h"
#include "RC/CharacterWriter.h"

class LimineTerminal : public CharacterWriter {
    u64 m_terminal_id;

    limine_terminal *current_terminal();

public:
    LimineTerminal() : m_terminal_id{0} {
        // TODO: assert terminal is not null
        // TODO: Assert terminal id exists
    }

    LimineTerminal(u64 terminal_id) : m_terminal_id{terminal_id} {
        // TODO: assert terminal is not null
        // TODO: Assert terminal id exists
    }

    static LimineTerminal init_default();

    int write(const char *str);
    int write_char(char c);
};
