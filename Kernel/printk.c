#include "printk.h"

#include <stdarg.h>

static int vfprintk(struct kernel_chardev device, const char *format,
                    va_list args);

int printk(const char *format, ...) {
    va_list args;
    va_start(args, format);
    int result = vfprintk(stdout, format, args);
    va_end(args);

    return result;
}

int debugk(const char *format, ...) {
    va_list args;
    va_start(args, format);
    int result = vfprintk(stddebug, format, args);
    va_end(args);

    return result;
}

static int vfprintk(struct kernel_chardev device, const char *format,
                    va_list args) {
    device.write(format);
    return 0;
}
