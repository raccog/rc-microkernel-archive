#pragma once

/**
 * Interface for reading characters from a device.
 */
class CharacterReader {
    // virtual const char* read_line();

    /**
     * Returns the next character read from the device.
     *
     * @return Next character.
     */
    virtual char read_char();
};
