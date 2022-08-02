/** \file
 * Kernel text output implementation.
 */

#include "terminal.h"

// Global limine text buffer
static struct limine_terminal *TERMINAL = NULL;
// Global limine text output
static struct limine_terminal_response *TERMINAL_RESPONSE = NULL;

void kernel_set_terminal(struct limine_terminal_response *terminal_response,
                         struct limine_terminal *terminal) {
    TERMINAL_RESPONSE = terminal_response;
    TERMINAL = terminal;
}

void kputc(char c) {
    if (TERMINAL_RESPONSE == NULL || TERMINAL == NULL)
        return;
    TERMINAL_RESPONSE->write(TERMINAL, &c, 1);
}

void kputs(const char *str) {
    if (TERMINAL_RESPONSE == NULL || TERMINAL == NULL)
        return;
    size_t len = strlen(str);
    TERMINAL_RESPONSE->write(TERMINAL, str, len);
}

u64 strlen(const char *str) {
    u64 len = 0;
    for (; *str != '\0'; ++str) {
        ++len;
    }
    return len;
}
