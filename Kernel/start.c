/** \file
 * Kernel start function.
 */

#include "External/limine.h"
#include "panic.h"
#include "segmentation.h"
#include "serial.h"
#include "terminal_limine.h"

#define KERNEL_VERSION "v0.0.1"
#define BOOT_MESSAGE                                                           \
    "rc-microkernel version (" KERNEL_VERSION ") compiled on " __DATE__        \
    " at " __TIME__ "\n"

/**
 * Kernel start function.
 */
void _start() {
    // init limine terminal
    terminal_limine_init();
    terminal_limine_write(BOOT_MESSAGE);

    // init serial device
    int error = serial_init();
    if (error != 0) {
        terminal_limine_write("FAILED: Serial initialization on port COM1!\n");
        panic();
    }
    serial_write(BOOT_MESSAGE);

    // Halt
    panic();
}
