/** \file
 * Kernel start function.
 */

#include "External/limine.h"
#include "segmentation.h"
#include "serial.h"
#include "terminal_limine.h"

#define KERNEL_VERSION "v0.0.1"
const char *HELLO = "Hi serial\n";

static void done() {
    for (;;) {
        __asm__("hlt");
    }
}

/**
 * Kernel start function.
 */
void _start() {
    // init limine terminal
    terminal_limine_init();
    terminal_limine_write("Hello limine terminal\nend\n");

    // init serial device
    int result = serial_init();
    if (result != 0) {
        terminal_limine_write("serial init failed\n");
        done();
    }

    serial_write("Hello serial\nend\n");
    // serial_write(HELLO);
    // serial_write_char('~');

    // Halt
    done();
}
