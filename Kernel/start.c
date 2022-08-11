#include "Arch/common.h"
#include "character_device.h"
#include "panic.h"
#include "printk.h"

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

    /* TODO: Start first process */
    panic();
}
