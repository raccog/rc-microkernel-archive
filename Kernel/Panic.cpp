#include "Panic.h"

void panic() {
    for (;;) {
        __asm__("hlt");
    }
}
