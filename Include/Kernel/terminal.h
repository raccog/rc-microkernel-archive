#pragma once

/**
 * Initializes a screen-based hardware terminal.
 *
 * @return Returns a status code. 0 if successful, -1 if failed.
 */
int terminal_init();

/**
 * Writes a string to the screen-based terminal.
 *
 * @return Returns a status code. 0 if successful, -1 if failed.
 */
int terminal_write(const char *str);

/**
 * Writes a single character to the screen-based terminal.
 *
 * @return Returns a status code. 0 if successful, -1 if failed.
 */
int terminal_write_char(char c);
