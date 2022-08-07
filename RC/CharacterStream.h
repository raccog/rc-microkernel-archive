/** \file
 * Interfaces for interacting with a character stream.
 */

#pragma once

class CharacterWriter {
    virtual int write_line(const char *str);
    virtual int write_char(char c);
};

class CharacterReader {
    //virtual const char* read_line();
    virtual char read_char(); 
};

class CharacterStream : public CharacterWriter, public CharacterReader {

};
