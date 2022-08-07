/** \file
 * Kernel start function.
 */

#include "External/limine.h"
#include "Segmentation.h"
#include "Serial.h"
#include "Stdint.h"
#include "Terminal.h"

constexpr const char *KERNEL_VERSION = "v0.0.1";

static volatile limine_terminal_request terminal_request = {
    LIMINE_TERMINAL_REQUEST, 0, 0, 0};

static void done() {
    for (;;) {
        __asm__("hlt");
    }
}

/**
 * Kernel start function.
 */
extern "C" void _start() {
    // Ensure terminal is available
    if (terminal_request.response == NULL ||
        terminal_request.response->terminal_count < 1) {
        done();
    }

    // Get limine terminal and test text output
    limine_terminal *terminal = terminal_request.response->terminals[0];

    // Test kernel text output
    kernel_set_terminal(terminal_request.response, terminal);
    kprintf("Starting rc-microkernel %s for x86-64...\n", KERNEL_VERSION);

    // set gdt
    // kprint_gdt();

    // init serial device
    kprintf("Serial: %i\n", kinit_serial());
    kputc_serial('~');

    // Halt
    done();
}
