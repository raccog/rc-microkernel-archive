/** \file
 * Kernel printf implementation.
 */

#include "Terminal.h"

#include <stdarg.h>
#include <stdbool.h>

// return the currently indexed character in the format string
// then increment the pointer to the next character
#define NEXT_CHAR() (*(format++))

// the printf format string.
// pointer can be moved forward by parsing functions
static const char *format;
// number of characters printed in printf
static int printed_chars;

// printf format specifier options
struct _format_specifier {
    bool force_sign;
    bool value_specifier;
    bool pad_sign;
    u32 precision;
    bool use_precision;
};

// parse printf format specifier flags
static void _parse_flags(struct _format_specifier *specifier) {
    while (*format != '\0') {
        switch (*format) {
        case '+':
            specifier->force_sign = true;
            break;
        case '#':
            specifier->value_specifier = true;
            break;
        case ' ':
            specifier->pad_sign = true;
            break;
        default:
            return;
        }
        ++format;
    }
}

// parse printf precision value
static void _parse_precision(va_list args,
                             struct _format_specifier *specifier) {
    // check for precision flag
    if (*format == '.') {
        specifier->use_precision = true;
        ++format;
        // get precision value from argument
        if (*format == '*') {
            specifier->precision = va_arg(args, u32);
            ++format;
            return;
        }
        // parse precision value digits
        // TODO: limit number of digits?
        char c = *format;
        while (c >= '0' && c <= '9') {
            specifier->precision *= 10;
            specifier->precision += (u32)(c - '0');
            ++format;
            c = *format;
        }
    }
}

static void _print_uint(u64 value, struct _format_specifier *specifier) {
    // print flag
    if (specifier->force_sign) {
        kputc('+');
        ++printed_chars;
    } else if (specifier->pad_sign) {
        kputc(' ');
        ++printed_chars;
    }

    // count digits
    u32 digits = 19;
    u64 compare = 1000000000000000000; // negative is used to compare
    while (digits > 1 && compare > value) {
        compare /= 10;
        --digits;
    }

    // pad with zeros if specified
    if (specifier->use_precision && specifier->precision > digits) {
        for (u32 i = 0; i < specifier->precision - digits; ++i) {
            kputc('0');
            ++printed_chars;
        }
    }

    // print integer digits
    while (compare > 0) {
        u64 digit = (value / compare) % 10;
        compare /= 10;
        kputc('0' + digit);
        ++printed_chars;
    }
}

static void _print_int(i64 value, struct _format_specifier *specifier) {
    // print flag
    if (value < 0) {
        kputc('-');
        ++printed_chars;
    } else if (specifier->force_sign) {
        kputc('+');
        ++printed_chars;
    } else if (specifier->pad_sign) {
        kputc(' ');
        ++printed_chars;
    }

    // value is compared as a negative integer because the minimum value would
    // not fit in the positive range
    if (value > 0) {
        value *= -1;
    }

    // count digits
    u32 digits = 19;
    i64 compare = -1000000000000000000; // negative is used to compare
    while (digits > 1 && compare < value) {
        compare /= 10;
        --digits;
    }

    // pad with zeros if specified
    if (specifier->use_precision && specifier->precision > digits) {
        for (u32 i = 0; i < specifier->precision - digits; ++i) {
            kputc('0');
            ++printed_chars;
        }
    }

    // print integer digits
    while (compare < 0) {
        i64 digit = (value / compare) % 10;
        compare /= 10;
        kputc('0' + digit);
        ++printed_chars;
    }
}

static void _print_hex(u64 value, struct _format_specifier *specifier) {
    // print hex specifier
    if (specifier->value_specifier) {
        kputs("0x");
        printed_chars += 2;
    }

    // count hex digits
    u32 shift = 60;
    if (value != 0) {
        while (((value >> shift) & 0xf) == 0) {
            shift -= 4;
        }
    } else {
        shift = 0;
    }

    // pad with zeros if necessary
    const u32 digits = (shift / 4) + 1; // 4 bits per digit
    if (specifier->precision > digits) {
        for (u32 i = 0; i < specifier->precision - digits; ++i) {
            kputc('0');
            ++printed_chars;
        }
    }

    // print hex digits
    while (shift >= 0) {
        u64 digit = (value >> shift) & 0xf;
        if (digit > 9) {
            kputc(digit - 0xa + 'a');
        } else {
            kputc(digit + '0');
        }
        ++printed_chars;
        if (shift > 0) {
            shift -= 4;
        } else {
            break;
        }
    }
}

// takes the next argument and formats it according to the current format
// specifier
static int _print_argument(va_list args) {
    // if "%%" is found in the format string, print a literal '%' character
    if (*format == '%') {
        kputc('%');
        ++format;
        ++printed_chars;
    } else if (*format != '\0') {
        // parse format specifier
        struct _format_specifier specifier = {.force_sign = false,
                                              .value_specifier = false,
                                              .pad_sign = false,
                                              .precision = 0,
                                              .use_precision = false};
        // Parse flags
        _parse_flags(&specifier);
        // Parse precision
        _parse_precision(args, &specifier);
        // Parse value specifier
        char value_specifier = *format;
        switch (value_specifier) {
        case 'i':
        case 'd':
            _print_int((i64)va_arg(args, i32), &specifier);
            break;
        case 'l':
            _print_int(va_arg(args, i64), &specifier);
            break;
        case 'u':
            _print_uint((u64)va_arg(args, u32), &specifier);
            break;
        case 'x':
            _print_hex(va_arg(args, u64), &specifier);
            break;
        case 'p':
            // always prefix pointers with "0x"
            specifier.value_specifier = true;
            _print_hex((u64)va_arg(args, const void *), &specifier);
            break;
        case 's':
            kputs((const char *)va_arg(args, const void *));
            break;
        case 'c':
            kputc(va_arg(args, int));
            break;
        default:
            // return -1 if specifier is invalid
            return -1;
        }
        ++format;
    } else {
        // return -1 if string is terminated before a format specifier
        return -1;
    }

    return 0;
}

int kprintf(const char *fmt, ...) {
    // start variable argument parsing
    va_list args;
    va_start(args, fmt);

    // set format string
    format = fmt;
    // count characters printed
    printed_chars = 0;

    while (*format != '\0') {
        char c = NEXT_CHAR();
        if (c == '%') {
            int result = _print_argument(args);
            if (result != 0)
                return result;
        } else {
            kputc(c);
            ++printed_chars;
        }
    }

    va_end(args);

    return printed_chars;
}
