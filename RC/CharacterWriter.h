#pragma once

/**
 * Interface for writing characters to a device.
 */
class CharacterWriter {
    /**
     * Writes a string of characters to this device.
     *
     * @param str String to write.
     */
    virtual int write(const char *str);

    /**
     * Writes a single character to this device.
     */
    virtual int write_char(char c);
};
