#include "String.h"

u64 strlen(const char *str) {
    u64 len = 0;
    for (; *str != '\0'; ++str) {
        ++len;
    }
    return len;
}
