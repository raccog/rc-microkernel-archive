/**
 * Implementation of `vfprintk()` is inspired by OpenBSD's `kprintf()`.
 *
 * A bunch of other implementation details are inspired by OpenBSD. :)
 */

#include "printk.h"

#include "RC/stdlib.h"
#include "RC/string.h"
#include <limits.h>
#include <stdarg.h>

/* most of printk is implemented here */
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

/* convert single base-10 digit integer to its character */
#define PRINTK_TO_CHAR(digit) ('0' + (char)(digit))

/* convert single base-10 digit integer from its character */
#define PRINTK_FROM_CHAR(character) (int)((character) - '0')

/* convert single base-16 digit integer to its hex character */
#define PRINTK_TO_HEX_CHAR(digit) (*(HEX_CHARS + (digit)))

/* which sign should be printed */
enum sign_config {
    SIGN_PLUS,   /* print a plus for positives */
    SIGN_SPACE,  /* print a space in place of a sign for positives */
    SIGN_DEFAULT /* print no sign for positives */
};

/* an integer value's length in bytes */
enum value_length {
    LENGTH_NONE,    /* int/unsigned int */
    LENGTH_SSHORT,  /* signed char/unsigned char */
    LENGTH_SHORT,   /* short int/short unsigned int */
    LENGTH_LONG,    /* long int/long unsigned int */
    LENGTH_LLONG,   /* long long int/long long unsigned int */
    LENGTH_MAX,     /* intmax_t/uintmax_t */
    LENGTH_SIZE,    /* size_t */
    LENGTH_POINTER, /* ptrdiff_t */
};

/* the type of value being formatted */
enum value_type {
    TYPE_CHAR,
    TYPE_STRING,
    TYPE_SIGNED_INTEGER,
    TYPE_UNSIGNED_INTEGER,
    TYPE_OCTAL_INTEGER,
    TYPE_HEX_INTEGER,
    TYPE_POINTER
};

static int vfprintk(struct kernel_chardev device, const char *format,
                    va_list args) {
    int count = 0;                   /* # of characters printed */
    char buffer[PRINTK_BUFFER_SIZE]; /* buffer for formatting values */
    const char *HEX_CHARS = "0123456789abcdef"; /* array of hex chars */

    /* value formatting state machine */
    char c;                   /* current character being parsed */
    const char *specifier;    /* start of specifier in format string */
    char *char_buf;           /* pointer to buffer */
    int char_buf_size;        /* length of buffer after value is formatted */
    int printed_digits;       /* number of digits in an integer value */
    enum sign_config sign;    /* sign to be printed */
    enum value_length length; /* length of integer value in bytes */
    enum value_type vtype;    /* type of value (int, char, string, pointer) */
    bool preceed_base;        /* should preceed integers with their radix */
    bool left_justify;        /* left-justifies the formatted value */
    bool pad_zeros;           /* pad with zeros instead of spaces */
    bool use_precision;       /* should precision be used */
    int width;                /* min width of value in characters */
    int precision;            /* min number of digits for integer value */
    u64 value;                /* unsigned value */
    i64 value_signed;         /* signed value */
    char value_char;          /* character value */
    const char *value_str;    /* string value */
    int __status;             /* status used for internal macro */

/* print char to device and increment counter. return if `write_char()` returns
 * an error. */
#define PRINTK_PUTC(character)                                                 \
    __status = device.write_char(character);                                   \
    if (__status < 0)                                                          \
        return __status;                                                       \
    ++count

/* print string to device and increment counter. return if `write()`
 * returns an error. */
#define PRINTK_PUTS(string)                                                    \
    __status = device.write(string);                                           \
    if (__status < 0)                                                          \
        return __status;                                                       \
    count += strlen(string)

    /* loop through format string */
    while (*format != '\0') {
        /* print all characters until a '%' is found */
        while (*format != '%' && *format != '\0') {
            /* process control is brought back here if parsing fails */
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
            /* preceed with base (0x or 0) */
            case '#':
                preceed_base = true;
                break;
            /* precision */
            case '.':
                /* cannot have multiple precicsion values */
                if (use_precision)
                    goto formatting_failed;
                use_precision = true;
                c = *specifier;
                /* get precision from argument */
                if (c == '*') {
                    precision = va_arg(args, int);
                    break;
                }
                /* parse precision */
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
                /* do not parse if padding was already specified */
                if (width != 0)
                    goto formatting_failed;
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
                /* do not parse if padding was already specified */
                if (width != 0)
                    goto formatting_failed;
                width = va_arg(args, int);
                break;
            /* short length value (or short-short) */
            case 'h':
                if (length == LENGTH_SHORT)
                    length = LENGTH_SSHORT;
                else if (length != LENGTH_NONE)
                    goto formatting_failed;
                else
                    length = LENGTH_SHORT;
                break;
            /* long length value (or long-long) */
            case 'l':
                if (length == LENGTH_LONG)
                    length = LENGTH_LLONG;
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
        /* pointer to end of buffer */
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
            case LENGTH_SSHORT:
                value_signed = (signed char)va_arg(args, int);
                break;
            case LENGTH_SHORT:
                value_signed = (short int)va_arg(args, int);
                break;
            case LENGTH_LONG:
                value_signed = va_arg(args, long int);
                break;
            case LENGTH_LLONG:
                value_signed = va_arg(args, long long int);
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
                case LENGTH_SSHORT:
                    value = (unsigned char)va_arg(args, unsigned int);
                    break;
                case LENGTH_SHORT:
                    value = (unsigned short int)va_arg(args, unsigned int);
                    break;
                case LENGTH_LONG:
                    value = va_arg(args, unsigned long int);
                    break;
                case LENGTH_LLONG:
                    value = va_arg(args, unsigned long long int);
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
                /* print octal */
                do {
                    *char_buf-- = PRINTK_TO_CHAR(value & 7);
                    value >>= 3;
                } while (value > 0);
                ++char_buf;
                break;
            case TYPE_UNSIGNED_INTEGER:
                /* print base-10 */
                while (value >= 10) {
                    *char_buf-- = PRINTK_TO_CHAR(value % 10);
                    value /= 10;
                }
                *char_buf = PRINTK_TO_CHAR(value % 10);
                break;
            case TYPE_HEX_INTEGER:
            case TYPE_POINTER:
                /* print hex */
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
        default:
            printed_digits = strlen(char_buf);
            char_buf_size = printed_digits;
            if (precision > char_buf_size)
                char_buf_size = precision;
        }

        /* count extra characters */
        switch (vtype) {
        case TYPE_SIGNED_INTEGER:
        case TYPE_UNSIGNED_INTEGER:
            /* preceeding sign */
            if (sign == SIGN_PLUS || sign == SIGN_SPACE ||
                (vtype == TYPE_SIGNED_INTEGER && value_signed < 0))
                ++char_buf_size;
            break;
        case TYPE_HEX_INTEGER:
        case TYPE_OCTAL_INTEGER:
        case TYPE_POINTER:
            /* preceeding radix */
            if (preceed_base) {
                if (vtype == TYPE_HEX_INTEGER)
                    char_buf_size += 2;
                else
                    ++char_buf_size;
            }
        default:
            break;
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
            if (vtype == TYPE_SIGNED_INTEGER) {
                if (value_signed < 0) {
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
                } else if (vtype == TYPE_OCTAL_INTEGER) {
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
            PRINTK_PUTS(value_str);
            break;
        case TYPE_SIGNED_INTEGER:
        case TYPE_UNSIGNED_INTEGER:
        case TYPE_HEX_INTEGER:
        case TYPE_POINTER:
        case TYPE_OCTAL_INTEGER:
            PRINTK_PUTS(char_buf);
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
