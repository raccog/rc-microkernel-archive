#include "panic.h"

void panic() {
    for (;;) {
        __asm__("hlt");
    }
}
