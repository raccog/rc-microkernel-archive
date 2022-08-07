/** \file
 * Kernel start function.
 */

#include "External/limine.h"
#include "LimineTerminal.h"
#include "Segmentation.h"
#include "Serial.h"

#include <limits.h>

constexpr const char *KERNEL_VERSION = "v0.0.1";

[[noreturn]] static void done() {
    for (;;) {
        __asm__("hlt");
    }
}

/**
 * Kernel start function.
 */
extern "C" void _start() {
    // init limine terminal
    LimineTerminal terminal = LimineTerminal::init_default();
    terminal.write("Hello limine terminal\nend\n");

    // init serial device
    Serial &serial = Serial::init_default();
    serial.write("Hello serial\nend\n");
    kserial_int(-1234567890);
    kserial_int(INT_MIN);
    kserial_int(LLONG_MIN);
    kserial_int(LLONG_MAX);
    kserial_int(0);

    // Halt
    done();
}
