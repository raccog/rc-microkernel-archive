/**
 * Implementation of `vfprintk()` is inspired by OpenBSD's `kprintf()`.
 *
 * A bunch of other implementation details are inspired by OpenBSD. :)
 */

#include "printk.h"

#include "Arch/x86_64/serial.h"
#include "string.h"
#include <limits.h>
#include <stdarg.h>

// TODO: Move to standard library
#define ABS(x) (x < 0 ? -x : x)

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
 * license has been included in the file `Attributions.md`.
 */

/*
 * Buffer should be able to hold a 64-bit integer that has been formatted into
 * a string with base-8. Also should include a null terminator.
 *
 * Straight from OpenBSD.
 */
#define PRINTK_BUFFER_SIZE (sizeof(i64) * CHAR_BIT / 3 + 2)

/* types of signs that can be printed */
enum sign_config {
    SIGN_PLUS,   /* print a plus for positives */
    SIGN_SPACE,  /* print a space in place of a sign for positives */
    SIGN_DEFAULT /* print no sign for positives */
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
    TYPE_UNSIGNED_INTEGER,
    TYPE_OCTAL_INTEGER,
    TYPE_HEX_INTEGER,
    TYPE_POINTER
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
    const char *specifier; /* start to specifier in format string */
    char c;                /* current character being parsed */
    char *char_buf;        /* pointer to buffer for next char */
    int char_buf_size;     /* length of formatted value including zeros/sign */
    int printed_digits;    /* length of formatted value */
    enum sign_config sign; /* what sign should be printed */
    enum value_length length; /* length of the value in bytes */
    enum value_type vtype;    /* type of the value (int, char, string) */
    bool preceed_base;        /* preceeds integers with their base */
    bool left_justify;        /* left-justifies the formatted value */
    bool pad_zeros;           /* pad with zeros instead of spaces */
    int width;                /* value padding width */
    bool use_precision;       /* whether precision should be used */
    int precision;            /* how precise the value should be */
    u64 value;                /* the value to format and print */
    i64 value_signed;         /* the signed value to format and print */
    char value_char;          /* the char to print */
    const char *value_str;    /* the string to print */

/* print char to device and increment counter */
#define PRINTK_PUTC(character)                                                 \
    device.write_char(character);                                              \
    ++count

#define PRINTK_TO_CHAR(digit) ('0' + (char)(digit))

#define PRINTK_TO_HEX_CHAR(digit) (*(HEX_CHARS + (digit)))

#define PRINTK_FROM_CHAR(character) (int)((character) - '0')

    /* loop through format string */
    while (*format != '\0') {
        /* print all characters until a '%' is found */
        while (*format != '%' && *format != '\0') {
            /* control is brought back here if formatting a value fails */
        formatting_failed:
            PRINTK_PUTC(*format++);
        }

        if (*format == '\0')
            break;

        /* move past the detected '%' */
        specifier = format + 1;

        /* literal '%' */
        if (*specifier == '%') {
            PRINTK_PUTC(*specifier++);
            continue;
        }

        /* initialize state machine */
        sign = SIGN_DEFAULT;
        length = LENGTH_NONE;
        preceed_base = false;
        left_justify = false;
        pad_zeros = false;
        width = 0;
        precision = 0;
        use_precision = false;

        /* handle value formatting */
        while (*specifier != '\0') {
            c = *specifier++;

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
                break;
            /* preceed with base (0x, 0o) */
            case '#':
                preceed_base = true;
                break;
            /* precision */
            case '.':
                if (use_precision)
                    goto formatting_failed;
                use_precision = true;
                c = *specifier;
                if (c == '*') {
                    precision = va_arg(args, int);
                    break;
                }
                /* count precision */
                while (c >= '0' && c <= '9') {
                    int digit = PRINTK_FROM_CHAR(c);
                    if (precision > (INT_MAX - digit) / 10)
                        goto formatting_failed;
                    precision = (precision * 10) + digit;
                    c = *++specifier;
                }
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
                /* do not format if padding was already specified */
                if (width != 0) {
                    goto formatting_failed;
                }
                /* count padding */
                --specifier;
                do {
                    int digit = PRINTK_FROM_CHAR(c);
                    if (width > (INT_MAX - digit) / 10)
                        goto formatting_failed;
                    width = (width * 10) + digit;
                    c = *++specifier;
                } while (c >= '0' && c <= '9');
                break;
            /* padding from variable arguments */
            case '*':
                /* do not format if padding was already specified */
                if (width != 0)
                    goto formatting_failed;
                width = va_arg(args, int);
                break;
            /* short length value (or short-short) */
            case 'h':
                if (length == LENGTH_SHORT)
                    length = LENGTH_SHORT_SHORT;
                else if (length != LENGTH_NONE)
                    goto formatting_failed;
                else
                    length = LENGTH_SHORT;
                break;
            /* long length value (or long-long) */
            case 'l':
                if (length == LENGTH_LONG)
                    length = LENGTH_LONG_LONG;
                else if (length != LENGTH_NONE)
                    goto formatting_failed;
                else
                    length = LENGTH_LONG;
                break;
            /* maximum integer length value */
            case 'j':
                if (length != LENGTH_NONE)
                    goto formatting_failed;
                else
                    length = LENGTH_MAX;
                break;
            /* size_t length value */
            case 'z':
                if (length != LENGTH_NONE)
                    goto formatting_failed;
                else
                    length = LENGTH_SIZE;
                break;
            /* pointer length value */
            case 't':
                if (length != LENGTH_NONE)
                    goto formatting_failed;
                else
                    length = LENGTH_POINTER;
                break;
            /* char value specifier */
            case 'c':
                vtype = TYPE_CHAR;
                goto format_value;
            /* string value specifier */
            case 's':
                vtype = TYPE_STRING;
                goto format_value;
            /* integer value specifier */
            case 'd':
            case 'i':
                vtype = TYPE_SIGNED_INTEGER;
                goto format_value;
            case 'u':
                vtype = TYPE_UNSIGNED_INTEGER;
                goto format_value;
            case 'p':
                vtype = TYPE_POINTER;
                preceed_base = true;
                goto format_value;
            case 'x':
            case 'X':
                vtype = TYPE_HEX_INTEGER;
                goto format_value;
            case 'o':
                vtype = TYPE_OCTAL_INTEGER;
                goto format_value;
            default:
                goto formatting_failed;
            }
        }
        if (*specifier == '\0')
            goto formatting_failed;

        /* format value into buffer */
    format_value:
        char_buf = buffer + PRINTK_BUFFER_SIZE - 1;
        *char_buf-- = '\0';
        switch (vtype) {
        case TYPE_CHAR:
            // TODO: wide char?
            value_char = (char)va_arg(args, int);
            break;
        case TYPE_STRING:
            // TODO: wide chars?
            value_str = va_arg(args, const char *);
            break;
        case TYPE_SIGNED_INTEGER:
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
                *char_buf-- = PRINTK_TO_CHAR(ABS(value_signed % 10));
                value_signed /= 10;
            }
            *char_buf = PRINTK_TO_CHAR(ABS(value_signed % 10));
            break;
        case TYPE_HEX_INTEGER:
        case TYPE_UNSIGNED_INTEGER:
        case TYPE_OCTAL_INTEGER:
        case TYPE_POINTER:
            /* interpret argument */
            if (vtype == TYPE_POINTER) {
                value = (u64)va_arg(args, void *);
            } else {
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
                    value = (u64)va_arg(args, size_t);
                    break;
                case LENGTH_POINTER:
                    value = (u64)va_arg(args, ptrdiff_t);
                    break;
                default:
                    value = va_arg(args, unsigned);
                }
            }
            /* print unsigned integer */
            switch (vtype) {
            case TYPE_OCTAL_INTEGER:
                do {
                    *char_buf-- = PRINTK_TO_CHAR(value & 7);
                    value >>= 3;
                } while (value > 0);
                ++char_buf;
                break;
            case TYPE_UNSIGNED_INTEGER:
                while (value >= 10) {
                    *char_buf-- = PRINTK_TO_CHAR(value % 10);
                    value /= 10;
                }
                *char_buf = PRINTK_TO_CHAR(value % 10);
                break;
            case TYPE_HEX_INTEGER:
            case TYPE_POINTER:
                do {
                    *char_buf-- = PRINTK_TO_HEX_CHAR(value & 0xf);
                    value >>= 4;
                } while (value > 0);
                ++char_buf;
                break;
            default:
                goto formatting_failed;
            }
        }

        /* calculate size of char buf */
        switch (vtype) {
        case TYPE_CHAR:
            char_buf_size = 1;
            break;
        case TYPE_STRING:
            char_buf_size = strlen(value_str);
            break;
        case TYPE_SIGNED_INTEGER:
        case TYPE_UNSIGNED_INTEGER:
            printed_digits = strlen(char_buf);
            char_buf_size = printed_digits;
            if (precision > char_buf_size)
                char_buf_size = precision;
            /* plus one for sign */
            if (sign == SIGN_PLUS || sign == SIGN_SPACE ||
                (vtype == TYPE_SIGNED_INTEGER && value_signed < 0))
                ++char_buf_size;
            break;
        case TYPE_HEX_INTEGER:
        case TYPE_OCTAL_INTEGER:
        case TYPE_POINTER:
            printed_digits = strlen(char_buf);
            char_buf_size = printed_digits;
            if (precision > char_buf_size)
                char_buf_size = precision;
            /* extra for preceeding base */
            if (preceed_base) {
                if (vtype == TYPE_HEX_INTEGER)
                    char_buf_size += 2;
                else
                    ++char_buf_size;
            }
        }

        /* add padding unless left-justified */
        if (!pad_zeros && !left_justify && width > char_buf_size) {
            for (int i = 0; i < width - char_buf_size; ++i) {
                PRINTK_PUTC(' ');
            }
        }

        /* integer-only formatting */
        switch (vtype) {
        case TYPE_SIGNED_INTEGER:
        case TYPE_UNSIGNED_INTEGER:
        case TYPE_HEX_INTEGER:
        case TYPE_OCTAL_INTEGER:
        case TYPE_POINTER:
            /* print sign if a base-10 integer */
            if (vtype == TYPE_SIGNED_INTEGER ||
                vtype == TYPE_UNSIGNED_INTEGER) {
                if (vtype == TYPE_SIGNED_INTEGER && value_signed < 0) {
                    PRINTK_PUTC('-');
                } else if (sign == SIGN_PLUS) {
                    PRINTK_PUTC('+');
                } else if (sign == SIGN_SPACE) {
                    PRINTK_PUTC(' ');
                }
                /* preceed with radix if base-8 or base-16 */
            } else if (preceed_base) {
                if (vtype == TYPE_HEX_INTEGER || vtype == TYPE_POINTER) {
                    PRINTK_PUTC('0');
                    PRINTK_PUTC('x');
                } else {
                    PRINTK_PUTC('0');
                }
            }

            /* add zeros for padding if needed */
            if (pad_zeros && width > char_buf_size) {
                for (int i = 0; i < width - char_buf_size; ++i) {
                    PRINTK_PUTC('0');
                }
            }

            /* add zeros for precision */
            if (use_precision && precision > printed_digits) {
                for (int i = 0; i < precision - printed_digits; ++i) {
                    PRINTK_PUTC('0');
                }
            }
            break;
        default:
            break;
        }

        /* print formatted value */
        switch (vtype) {
        case TYPE_CHAR:
            PRINTK_PUTC(value_char);
            break;
        case TYPE_STRING:
            device.write(value_str);
            break;
        case TYPE_SIGNED_INTEGER:
        case TYPE_UNSIGNED_INTEGER:
        case TYPE_HEX_INTEGER:
        case TYPE_POINTER:
        case TYPE_OCTAL_INTEGER:
            device.write(char_buf);
        }

        /* pad if left justified */
        if (!pad_zeros && left_justify && width > char_buf_size) {
            for (int i = 0; i < width - char_buf_size; ++i) {
                PRINTK_PUTC(' ');
            }
        }

        /* move position in format string */
        format = specifier;
    }

    return count;
}
