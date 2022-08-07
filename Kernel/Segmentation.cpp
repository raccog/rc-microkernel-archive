/** \file
 * Segmentation functions.
 */

#include "Segmentation.h"

void kprint_gdt() {
    struct gdt_register gdtr = kget_gdt();
    struct segment_descriptor *gdt = gdtr.base;
    u16 limit = gdtr.limit;
}
