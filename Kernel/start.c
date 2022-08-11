#include "External/limine.h"
#include "character_device.h"
#include "panic.h"
#include "printk.h"
#include "segmentation.h"
#include "serial.h"
#include "terminal.h"

#include <limits.h>

#define KERNEL_VERSION "v0.0.1"
#define BOOT_MESSAGE                                                           \
    "rc-microkernel version (" KERNEL_VERSION ") compiled on " __DATE__        \
    " at " __TIME__ "\n"

/**
 * Kernel start function.
 */
void _start() {
    /* Initialize 2 character output devices */
    terminal_init();
    serial_init();
    fallback_chardev_init();

    /*
     * NOTE
     * printk() and debugk() can ONLY be used after this line.
     */

    /* Print boot message */
    printk(BOOT_MESSAGE);
    /* Only print to stddebug if it is a separate device from stdout */
    if (stdout.write != stddebug.write) {
        debugk(BOOT_MESSAGE);
    }

    // Halt
    panic();
}
