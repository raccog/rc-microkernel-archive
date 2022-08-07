#pragma once

#include "External/limine.h"
#include "RC/CharacterWriter.h"

/**
 * Limine terminal output.
 */
class LimineTerminal : public CharacterWriter {
    u64 m_terminal_id;

    limine_terminal *current_terminal();

public:
    /**
     * Use terminal with id 0 by default.
     */
    LimineTerminal() : m_terminal_id{0} {
        // TODO: assert terminal is not null
        // TODO: Assert terminal id exists
    }

    /**
     * Use terminal with specified id.
     *
     * @param terminal_id Terminal id to use.
     */
    LimineTerminal(u64 terminal_id) : m_terminal_id{terminal_id} {
        // TODO: assert terminal is not null
        // TODO: Assert terminal id exists
    }

    /**
     * Returns the default Limine terminal.
     *
     * Panics if terminal could not be found.
     *
     * @return The default Limine terminal.
     */
    static LimineTerminal init_default();

    int write(const char *str);
    int write_char(char c);
};
