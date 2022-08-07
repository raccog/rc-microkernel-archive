#pragma once

class CharacterWriter {
    virtual int write(const char *str);
    virtual int write_char(char c);
};
