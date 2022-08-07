/** \file
 * Kernel text output functions.
 */

#pragma once

#include "External/limine.h"
#include "Stdint.h"

/**
 * Sets a limine terminal as global kernel stdout.
 *
 * @param terminal_response Limine terminal response contains a write() function
 *                          to output text.
 * @param terminal Limine text buffer.
 */
void kernel_set_terminal(struct limine_terminal_response *terminal_response,
                         struct limine_terminal *terminal);

/**
 * Writes a formatted string to the limine terminal using the stack to buffer
 * output.
 *
 * The formatting string can contain format specifiers similar to C-style
 * printf format specifiers. Unlike C-style format specifiers, there cannot
 * be a width value.
 *
 * Each format specifier should follow this syntax:
 *
 * `%[flags][.precision]type`
 *
 * The following flags can be used:
 *  - `+`
 *  Adds a `+` character to the start of positive numbers.
 *  - `<SPACE>`
 *  Adds a space to the start of positive numbers.
 *  - `#`
 *  Adds an explicit formatting symbol to the variable. For base-10 integers, 0
 *  is inserted after the sign. For base-16 integers, 0x is inserted after the
 *  sign. For decimal numbers, the decimal point is always printed, even if
 *  there is no fractional value.
 *
 * @param format The formatting string.
 * @param ... A sequence of variables to be formatted. The
 *            sequence should be in the same order specified in `format`.
 *            There should be at least as many variables in the sequence as
 *            there are format specifiers in `format`.
 * @return When successful, the number of characters written is returned.
 *         When failed, `-1` is returned.
 */
int kprintf(const char *format, ...);

/**
 * Writes a character to the limine terminal.
 *
 * @param c Character to output.
 */
void kputc(char c);

/**
 * Writes a null-terminated string to the limine terminal.
 *
 * @param str Null-terminated string to output.
 */
void kputs(const char *str);

/**
 * Get the length of a null-terminated string.
 *
 * @param str A null-terminated string.
 * @return Length of `str`.
 */
u64 strlen(const char *str);
