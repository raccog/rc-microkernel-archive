/** \file
 * Kernel start function.
 */

#include "limine.h"
#include "stdint.h"
#include "terminal.h"

#define KERNEL_VERSION "v0.0.1"

static volatile struct limine_terminal_request terminal_request = {
    LIMINE_TERMINAL_REQUEST, 0, 0, 0};

static void done() {
    for (;;) {
        __asm__("hlt");
    }
}

#include <limits.h>

/**
 * Kernel start function.
 */
void _start() {
    // Ensure terminal is available
    if (terminal_request.response == NULL ||
        terminal_request.response->terminal_count < 1) {
        done();
    }

    // Get limine terminal and test text output
    struct limine_terminal *terminal = terminal_request.response->terminals[0];

    // Test kernel text output
    kernel_set_terminal(terminal_request.response, terminal);
    kprintf("Starting rc-microkernel %s for x86-64...", KERNEL_VERSION);

    // Halt
    done();
}
