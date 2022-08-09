#include "terminal_limine.h"

#include "External/limine.h"
#include "RC/string.h"
#include "panic.h"

// TODO: Move to limine elf section
static volatile struct limine_terminal_request TERMINAL_REQUEST = {
    .id = LIMINE_TERMINAL_REQUEST, .revision = 0};

static struct limine_terminal *TERMINAL0 = NULL;

void terminal_limine_init() {
    if (TERMINAL0)
        return;

    // ensure terminal is available
    if (!TERMINAL_REQUEST.response || !TERMINAL_REQUEST.response->terminals ||
        TERMINAL_REQUEST.response->terminal_count < 1)
        panic();

    TERMINAL0 = TERMINAL_REQUEST.response->terminals[0];
}

int terminal_limine_write(const char *str) {
    u64 len = strlen(str);
    if (!TERMINAL0)
        return -1;
    TERMINAL_REQUEST.response->write(TERMINAL0, str, len);
    return 0;
}

int terminal_limine_write_char(char c) {
    if (!TERMINAL0)
        return -1;
    TERMINAL_REQUEST.response->write(TERMINAL0, &c, 1);
    return 0;
}
