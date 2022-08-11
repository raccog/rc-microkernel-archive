#ifdef BOOTLOADER_LIMINE

#include "terminal.h"

#include "External/limine.h"
#include "RC/string.h"
#include "character_device.h"
#include "panic.h"

// TODO: Move to limine elf section
static volatile struct limine_terminal_request TERMINAL_REQUEST = {
    .id = LIMINE_TERMINAL_REQUEST, .revision = 0};

static struct limine_terminal *TERMINAL0 = NULL;

int terminal_init() {
    // ensure terminal is available
    if (!TERMINAL_REQUEST.response || !TERMINAL_REQUEST.response->terminals ||
        TERMINAL_REQUEST.response->terminal_count < 1)
        return -1;

    TERMINAL0 = TERMINAL_REQUEST.response->terminals[0];

    stdout = (struct kernel_chardev){.write = &terminal_write,
                                     .write_char = &terminal_write_char};

    return 0;
}

int terminal_write(const char *str) {
    u64 len = strlen(str);
    if (!TERMINAL0)
        return -1;
    TERMINAL_REQUEST.response->write(TERMINAL0, str, len);
    return 0;
}

int terminal_write_char(char c) {
    if (!TERMINAL0)
        return -1;
    TERMINAL_REQUEST.response->write(TERMINAL0, &c, 1);
    return 0;
}

#endif
