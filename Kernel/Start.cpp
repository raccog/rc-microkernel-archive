/** \file
 * Kernel start function.
 */

#include "External/limine.h"
#include "LimineTerminal.h"
#include "Segmentation.h"
#include "Serial.h"
#include "Stdint.h"
#include "Terminal.h"

constexpr const char *KERNEL_VERSION = "v0.0.1";

static void done() {
    for (;;) {
        __asm__("hlt");
    }
}

/**
 * Kernel start function.
 */
extern "C" void _start() {
    // set gdt
    // kprint_gdt();

    // init limine terminal
    LimineTerminal terminal = LimineTerminal::default_terminal();
    terminal.write("Hello limine terminal\nend");

    // init serial device
    Serial serial = Serial::default_serial();
    serial.write("Hello serial\nend");

    // Halt
    done();
}
