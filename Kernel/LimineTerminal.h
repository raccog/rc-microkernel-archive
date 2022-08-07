#pragma once

#include "External/limine.h"
#include "RC/CharacterWriter.h"

class LimineTerminal : public CharacterWriter {
public:
    int write(const char *str) {}

    int write_char(char c) {}

private:
    const limine_terminal *m_terminal;
};
