#include "Arch/common.h"
#include "character_device.h"
#include "panic.h"
#include "printk.h"

#include <limits.h>

#define KERNEL_VERSION "v0.0.1"
#define BOOT_MESSAGE                                                           \
    "rc-microkernel version (" KERNEL_VERSION ") compiled on " __DATE__        \
    " at " __TIME__ "\n"

/*
 * Print a boot message to stdout.
 */
static void print_boot_message() {
    printk(BOOT_MESSAGE);
    /* Only print to stddebug if it is a separate device from stdout */
    if (stdout.write != stddebug.write) {
        debugk(BOOT_MESSAGE);
    }
}

/**
 * Kernel start function.
 */
void _start() {
    arch_early_init();

    /*
     * NOTE
     * printk() and debugk() can ONLY be used after arch_early_init() is
     * called.
     */

    print_boot_message();

    arch_init();

    printk("Hello %s number %i %x %c?\n", "World", -1234567890, 0xdeadbeef,
           '~');
    printk("INTMAX %i INTMIN %i\n", INT_MAX, INT_MIN);
    printk("INT0 %i\n", 0);
    printk("HEX %#x %x\n", 0xffffffff, 0);
    debugk("OCTAL %#llo %o\n", LLONG_MAX, 0);
    printk("%12i\n", 1234567890);
    printk("%p\n", (void *)0x12345678deadbeef);

    /* TODO: Start first process */
    panic();
}
