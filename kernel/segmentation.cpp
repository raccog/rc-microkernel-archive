/** \file
 * Segmentation functions.
 */

#include "segmentation.h"

#include "terminal.h"

void kprint_gdt() {
    struct gdt_register gdtr = kget_gdt();
    struct segment_descriptor *gdt = gdtr.base;
    u16 limit = gdtr.limit;
    kprintf("GDTR [base]: %.16p, [limit]: %#x\n", gdtr.base, limit);
    kprintf("end");
}
