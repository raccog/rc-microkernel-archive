#ifdef ARCH_x86_64

#include "Arch/common.h"

#include "Arch/x86_64/gdt.h"
#include "Arch/x86_64/idt.h"
#include "Arch/x86_64/serial.h"
#include "character_device.h"
#include "terminal.h"

void arch_early_init() {
    /* Initialize 2 character output devices */
    terminal_init();
    serial_init();
    fallback_chardev_init();
}

void arch_init() {
    init_gdt();
    init_idt();
}

#endif
