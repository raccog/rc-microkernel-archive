/** \file
 * Segmentation functions.
 */

#include "segmentation.h"

#include "terminal.h"

void kprint_gdt() {
    const struct segment_descriptor *gdt = 0;
    asm("sgdt [%0]" : "=r"(gdt));
    kprintf("GDT at: %p\n", gdt);
}
