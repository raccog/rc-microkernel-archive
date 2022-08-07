#include "LimineTerminal.h"

#include "Panic.h"
#include "Terminal.h"

// TODO: Move to limine elf section
// extern "C"
static volatile limine_terminal_request TERMINAL_REQUEST = {
    LIMINE_TERMINAL_REQUEST, 0, 0, 0};

LimineTerminal LimineTerminal::default_terminal() {
    // Ensure terminal is available
    if (TERMINAL_REQUEST.response == nullptr ||
        TERMINAL_REQUEST.response->terminal_count < 1) {
        panic();
    }

    return LimineTerminal(0);
}

int LimineTerminal::write(const char *str) {
    u64 len = strlen(str);
    limine_terminal *terminal = current_terminal();
    if (!terminal)
        return -1;
    TERMINAL_REQUEST.response->write(terminal, str, len);
    return 0;
}

int LimineTerminal::write_char(char c) {
    limine_terminal *terminal = current_terminal();
    if (!terminal)
        return -1;
    TERMINAL_REQUEST.response->write(terminal, &c, 1);
    return 0;
}

limine_terminal *LimineTerminal::current_terminal() {
    return TERMINAL_REQUEST.response->terminals[m_terminal_id];
}
