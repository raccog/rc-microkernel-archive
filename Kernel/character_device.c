#include "character_device.h"

#include "panic.h"

struct kernel_chardev stdout = {.write = NULL, .write_char = NULL};

struct kernel_chardev stddebug = {.write = NULL, .write_char = NULL};

static bool is_valid_chardev(struct kernel_chardev device) {
    return (device.write && device.write_char);
}

void fallback_chardev_init() {
    /* Panic if there is no valid character device */
    if (!is_valid_chardev(stdout) && !is_valid_chardev(stddebug))
        panic();
    /* Change text output stream to second device if one failed to
     * initialize */
    else if (!is_valid_chardev(stdout))
        stdout = stddebug;
    else if (!is_valid_chardev(stddebug))
        stddebug = stdout;
}
