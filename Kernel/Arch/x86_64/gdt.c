#ifdef ARCH_x86_64

#include "Arch/x86_64/gdt.h"

#include "printk.h"

void init_gdt() {
    struct region_descriptor *gdtr = get_gdt();
    struct mem_segment_descriptor *descriptors =
        (struct mem_segment_descriptor *)gdtr->base;
    size_t num_descriptors =
        gdtr->limit / sizeof(struct mem_segment_descriptor);
    debugk("GDT at address %p with %zu descriptors.\n", (void *)gdtr->base,
           num_descriptors);
    for (size_t i = 0; i < num_descriptors; ++i) {
        struct mem_segment_descriptor *desc = &descriptors[i + 1];
        debugk("GDT[%zu]: base=%#x, limit=%#x, type=%i, dpl=%i, present=%i, "
               "available=%i, long_mode=%i, def32=%i, gran=%i\n",
               i + 1, (desc->hibase << 24) | desc->lobase,
               (desc->hilimit << 16) | desc->lolimit, desc->sd_type, desc->dpl,
               desc->p, desc->available, desc->long_mode, desc->def32,
               desc->gran);
    }
}

#endif
