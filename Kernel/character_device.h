#pragma once

struct kernel_chardev {
    int (*write)(const char *str);
    int (*write_char)(char c);
};

extern struct kernel_chardev stdout;
extern struct kernel_chardev stddebug;

void fallback_chardev_init();
