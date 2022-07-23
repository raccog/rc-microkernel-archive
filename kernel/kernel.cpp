#include <stdint.h>
#include <stddef.h>
#include "limine.h"

static volatile struct limine_terminal_request terminal_request = {
    LIMINE_TERMINAL_REQUEST,
    0,
    0,
    0
};

static void done(void) {
    for (;;) {
        __asm__("hlt");
    }
}

extern "C" void _start(void) {
    // Ensure terminal is available
    if (terminal_request.response == NULL
            || terminal_request.response->terminal_count < 1) {
        done();
    }

    struct limine_terminal *terminal = terminal_request.response->terminals[0];
    terminal_request.response->write(terminal, "Hello warld", 11);

    done();

}

