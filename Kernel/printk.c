/**
 * Implementation of `vfprintk()` is inspired by OpenBSD's `kprintf()`.
 *
 * A bunch of other implementation details are inspired by OpenBSD. :)
 */

#include "printk.h"

#include "string.h"
#include <limits.h>
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

/*
 * Everything past this point includes modified snippets from OpenBSD. Code has
 * been written from scratch, with some snippets and implementation techniques
 * originating from OpenBSD in the file `sys/kern/subr_prf.c`. OpenBSD's
 * license has been included in the file `Attribution.md`.
 */

#define CHAR_BITS 8

/* Buffer should be able to hold a 64-bit integer that has been formatted into
 * a string with base-8. Also should include a null terminator. */
#define PRINTK_BUFFER_SIZE (sizeof(i64) * CHAR_BITS / 3 + 2)

/* types of signs that can be printed */
enum sign_config {
    SIGN_PLUS,   /* print a plus for positives */
    SIGN_SPACE,  /* print a space in place of a sign for positives */
    SIGN_DEFAULT /* print no sign for positives */
};

/* the numerical radix of the value being formatted */
enum value_radix {
    BASE_8,  /* base-8 (octal) */
    BASE_10, /* base-10 (decimal) */
    BASE_16  /* base-16 (hexadecimal) */
};

/* the length in bytes of a value */
enum value_length {
    LENGTH_NONE,
    LENGTH_SHORT_SHORT,
    LENGTH_SHORT,
    LENGTH_LONG,
    LENGTH_LONG_LONG,
    LENGTH_MAX,
    LENGTH_SIZE,
    LENGTH_POINTER,
};

/* the type of the value */
enum value_type {
    TYPE_CHAR,
    TYPE_STRING,
    TYPE_SIGNED_INTEGER,
    TYPE_UNSIGNED_INTEGER
};

// TODO: Record error codes
#define PRINTK_PLUS_OVERWRITTEN (-1)
#define PRINTK_PADDING_OVERFLOW (-2)
#define PRINTK_MULTIPLE_PADDINGS (-3)
#define PRINTK_INVALID_VALUE_LENGTH (-4)
#define PRINTK_INVALID_FORMATTING (-5)
#define PRINTK_PREMATURE_TERMINATOR (-6)

static int vfprintk(struct kernel_chardev device, const char *format,
                    va_list args) {
    int count = 0;                   /* # of characters printed */
    char buffer[PRINTK_BUFFER_SIZE]; /* buffer for formatting values */
    const char *HEX_CHARS = "0123456789abcdef"; /* array of hex chars */

    /* value formatting state machine */
    char c;                   /* current character being parsed */
    char *next_char;          /* pointer to buffer for next char */
    size_t buffered_chars;    /* number of characters put in a buffer */
    enum sign_config sign;    /* what sign should be printed */
    bool preceed_base;        /* preceeds integers with their base */
    bool left_justify;        /* left-justifies the formatted value */
    bool pad_zeros;           /* pad with zeros instead of spaces */
    int width;                /* value padding width */
    enum value_radix radix;   /* the value's radix (8, 10, or 16) */
    u64 value;                /* the value to format and print */
    i64 value_signed;         /* the signed value to format and print */
    char value_char;          /* the char to print */
    const char *value_str;    /* the string to print */
    bool found_specifier;     /* true when d,i,u,o,x,X,c,s,p is found */
    enum value_length length; /* length of the value in bytes */
    enum value_type vtype;    /* type of the value (int, char, string) */

/* prints a character to the device and increments the counter */
// TODO: detect counter overflow
#define VFPRINTK_CHAR(character)                                               \
    device.write_char(character);                                              \
    ++count;

    while (*format != '\0') {
        /* print all characters until a '%' is found */
        while (*format != '%' && *format != '\0') {
            VFPRINTK_CHAR(*format++);
        }
        if (*format == '\0')
            break;

        /* move past the detected '%' */
        ++format;

        /* initialize state machine */
        sign = SIGN_DEFAULT;
        radix = BASE_10;
        length = LENGTH_NONE;
        preceed_base = false;
        left_justify = false;
        pad_zeros = false;
        width = 0;
        found_specifier = false;

        /* handle value formatting */
        while (*format != '\0') {
            c = *format++;

            /* literal '%' character */
            if (c == '%') {
                VFPRINTK_CHAR(c);
                break;
            }

            switch (c) {
            /* left justify */
            case '-':
                left_justify = true;
                break;
            /* force plus sign */
            case '+':
                sign = SIGN_PLUS;
                break;
            /* space instead of sign */
            case ' ':
                /* do not overwrite plus sign */
                if (sign != SIGN_PLUS)
                    sign = SIGN_SPACE;
                else
                    // Plus sign overwritten by a space
                    // TODO: Error code
                    return PRINTK_PLUS_OVERWRITTEN;
                break;
            /* preceed with base (0x, 0o) */
            case '#':
                preceed_base = true;
                break;
            /* pad with zeros instead of spaces */
            case '0':
                pad_zeros = true;
                break;
            /* padding digits */
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                if (width != 0)
                    // Multiple paddings
                    // TODO: Error code
                    return PRINTK_MULTIPLE_PADDINGS;
                do {
                    int digit = (int)(c - '0');
                    if (width > (INT_MAX - digit) / 10)
                        // Padding overflow
                        // TODO: Error code
                        return PRINTK_PADDING_OVERFLOW;
                    width = (width * 10) + digit;
                    c = *format++;
                } while (c >= '0' && c <= '9');
                break;
            /* padding from variable arguments */
            case '*':
                if (width != 0)
                    // Multiple paddings
                    // TODO: Error code
                    return PRINTK_MULTIPLE_PADDINGS;
                width = va_arg(args, int);
                break;
            /* short length value (or short-short) */
            case 'h':
                if (length == LENGTH_SHORT)
                    length = LENGTH_SHORT_SHORT;
                else if (length != LENGTH_NONE)
                    // Too many length specifiers
                    // TODO: Error code
                    return PRINTK_INVALID_VALUE_LENGTH;
                else
                    length = LENGTH_SHORT;
                break;
            /* long length value (or long-long) */
            case 'l':
                if (length == LENGTH_LONG)
                    length = LENGTH_LONG_LONG;
                else if (length != LENGTH_NONE)
                    // Too many length specifiers
                    // TODO: Error code
                    return PRINTK_INVALID_VALUE_LENGTH;
                else
                    length = LENGTH_LONG;
                break;
            /* maximum integer length value */
            case 'j':
                if (length != LENGTH_NONE)
                    // Too many length specifiers
                    // TODO: Error code
                    return PRINTK_INVALID_VALUE_LENGTH;
                else
                    length = LENGTH_MAX;
                break;
            /* size_t length value */
            case 'z':
                if (length != LENGTH_NONE)
                    // Too many length specifiers
                    // TODO: Error code
                    return PRINTK_INVALID_VALUE_LENGTH;
                else
                    length = LENGTH_SIZE;
                break;
            /* pointer length value */
            case 't':
                if (length != LENGTH_NONE)
                    // Too many length specifiers
                    // TODO: Error code
                    return PRINTK_INVALID_VALUE_LENGTH;
                else
                    length = LENGTH_POINTER;
                break;
            /* char value specifier */
            case 'c':
                vtype = TYPE_CHAR;
                found_specifier = true;
                break;
            /* string value specifier */
            case 's':
                vtype = TYPE_STRING;
                found_specifier = true;
                break;
            /* integer value specifier */
            case 'd':
            case 'i':
                vtype = TYPE_SIGNED_INTEGER;
                radix = BASE_10;
                found_specifier = true;
                break;
            case 'u':
                vtype = TYPE_UNSIGNED_INTEGER;
                radix = BASE_10;
                found_specifier = true;
                break;
            case 'x':
            case 'X':
            case 'p':
                vtype = TYPE_UNSIGNED_INTEGER;
                radix = BASE_16;
                found_specifier = true;
                break;
            case 'o':
                vtype = TYPE_UNSIGNED_INTEGER;
                radix = BASE_8;
                found_specifier = true;
                break;
            default:
                // Invalid formatting string
                // TODO: Error code?
                return PRINTK_INVALID_FORMATTING;
            }

            if (found_specifier) {
                /* format value into buffer */
                next_char = buffer + PRINTK_BUFFER_SIZE - 1;
                *next_char-- = '\0';
                buffered_chars = 0;
                switch (vtype) {
                case TYPE_CHAR:
                    // TODO: wide char?
                    value_char = (char)va_arg(args, int);
                    buffered_chars = 1;
                    break;
                case TYPE_STRING:
                    // TODO: wide chars?
                    value_str = va_arg(args, const char *);
                    buffered_chars = strlen(value_str);
                    break;
                case TYPE_SIGNED_INTEGER:
                    buffered_chars = 0;
                    /* interpret argument */
                    switch (length) {
                    case LENGTH_SHORT_SHORT:
                        value_signed = (signed char)va_arg(args, int);
                        break;
                    case LENGTH_SHORT:
                        value_signed = (short)va_arg(args, int);
                        break;
                    case LENGTH_LONG:
                        value_signed = va_arg(args, long);
                        break;
                    case LENGTH_LONG_LONG:
                        value_signed = va_arg(args, long long);
                        break;
                    case LENGTH_MAX:
                        value_signed = va_arg(args, intmax_t);
                        break;
                    case LENGTH_SIZE:
                        value_signed = (i64)va_arg(args, size_t);
                        break;
                    case LENGTH_POINTER:
                        value_signed = (i64)va_arg(args, ptrdiff_t);
                        break;
                    default:
                        value_signed = va_arg(args, int);
                    }
                    /* print signed integer */
                    while (value_signed >= 10 || value_signed <= -10) {
                        *next_char-- = '0' + (char)(value_signed % 10);
                        ++buffered_chars;
                        value_signed /= 10;
                    }
                    *next_char = '0' + (char)(value_signed % 10);
                    ++buffered_chars;
                    if (value_signed < 0) {
                        ++buffered_chars;
                        *--next_char = '-';
                    }
                    break;
                case TYPE_UNSIGNED_INTEGER:
                    buffered_chars = 0;
                    /* interpret argument */
                    switch (length) {
                    case LENGTH_SHORT_SHORT:
                        value = (unsigned char)va_arg(args, unsigned);
                        break;
                    case LENGTH_SHORT:
                        value = (unsigned short)va_arg(args, unsigned);
                        break;
                    case LENGTH_LONG:
                        value = va_arg(args, unsigned long);
                        break;
                    case LENGTH_LONG_LONG:
                        value = va_arg(args, unsigned long long);
                        break;
                    case LENGTH_MAX:
                        value = va_arg(args, uintmax_t);
                        break;
                    case LENGTH_SIZE:
                        value = (i64)va_arg(args, size_t);
                        break;
                    case LENGTH_POINTER:
                        value = (i64)va_arg(args, ptrdiff_t);
                        break;
                    default:
                        value = va_arg(args, unsigned);
                    }
                    /* print unsigned integer */
                    switch (radix) {
                    case BASE_8:
                        do {
                            *next_char-- = '0' + (char)(value & 7);
                            ++buffered_chars;
                            value >>= 3;
                        } while (value > 0);
                        break;
                    case BASE_10:
                        while (value >= 10) {
                            *next_char-- = '0' + (char)(value % 10);
                            ++buffered_chars;
                            value /= 10;
                        }
                        *next_char = '0' + (char)(value_signed % 10);
                        ++buffered_chars;
                        break;
                    case BASE_16:
                        do {
                            *next_char-- = *(HEX_CHARS + (char)(value & 0xf));
                            ++buffered_chars;
                            value >>= 4;
                        } while (value > 0);
                    }
                }

                switch (vtype) {
                case TYPE_CHAR:
                    VFPRINTK_CHAR((char)va_arg(args, int));
                    break;
                case TYPE_STRING:
                    device.write(value_str);
                    break;
                case TYPE_SIGNED_INTEGER:
                case TYPE_UNSIGNED_INTEGER:
                    device.write(next_char);
                }
            }
        }
        if (*format == '\0')
            // Premature null terminator
            // TODO: Error code?
            return PRINTK_PREMATURE_TERMINATOR;
    }

    return 0;
}
#undef VFPRINTK_CHAR
