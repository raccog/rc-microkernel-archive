/** \file
 * Kernel start function.
 */

#include "External/limine.h"
#include "panic.h"
#include "segmentation.h"
#include "serial.h"
#include "terminal_limine.h"

#include <limits.h>

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

    serial_write("Signed integers:\n");
    serial_write_int(0);
    serial_write_int(-1234567890);
    serial_write_int(LLONG_MAX);
    serial_write_int(LLONG_MIN);

    serial_write("Unsigned integers:\n");
    serial_write_uint(0);
    serial_write_uint(1234567890);
    serial_write_uint(ULONG_MAX);

    serial_write("Hex integers:\n");
    serial_write_hex(0xdeadbeef);
    serial_write_hex(0xdeadbeefdeadbeef);
    serial_write_hex(0x0);
    serial_write_hex(0xffffffffffffffff);

    // Halt
    panic();
}
