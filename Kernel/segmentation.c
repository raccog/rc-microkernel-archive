#include "segmentation.h"

void x86_64_print_gdt() {
    struct x86_64_gdt_register gdtr = x86_64_get_gdt();
    struct x86_64_segment_descriptor *gdt = gdtr.base;
    u16 limit = gdtr.limit;
}
