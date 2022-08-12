#pragma once

/**
 * A simple character output device for use in the kernel.
 *
 * Contains function pointers for outputing text to an abstract device.
 */
struct kernel_chardev {
    /**
     * Write a string of characters to the device.
     */
    int (*write)(const char *str);

    /**
     * Write a single character to the device.
     */
    int (*write_char)(char c);
};

/**
 * The character device used for standard output.
 *
 * By default it is set to a screen-based terminal.
 */
extern struct kernel_chardev stdout;

/**
 * The character device used for debug output.
 *
 * By default it is set to a serial line.
 */
extern struct kernel_chardev stddebug;

/**
 * Initializes stdout or stddebug if they failed to initialize the first time.
 *
 * Panics if both character devices failed to initialize.
 */
void fallback_chardev_init();
