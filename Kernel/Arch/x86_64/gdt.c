#ifdef ARCH_x86_64

#include "Arch/x86_64/gdt.h"

#include "printk.h"

/* 256 GDT descriptors + 1 null descriptor */
#define GDT_LEN (256 + 1)
#define GDT_SIZE (sizeof(struct mem_segment_descriptor) * GDT_LEN)
#define GDT_LIMIT (GDT_SIZE - sizeof(struct mem_segment_descriptor))

struct mem_segment_descriptor gdt[GDT_LEN];

const char *bit_set_string(unsigned int bit) {
    if (bit)
        return " true";
    else
        return "false";
}

void *memcpy(void *dest, const void *src, size_t count) {
    u8 *dest_bytes = (u8 *)dest;
    const u8 *src_bytes = (const u8 *)src;
    for (size_t i = 0; i < count; ++i) {
        dest_bytes[i] = src_bytes[i];
    }
    return dest;
}

void init_gdt() {
    /* get GDT from bootloader */
    struct region_descriptor *gdtr = get_gdt();
    struct mem_segment_descriptor *bootloader_descriptors =
        (struct mem_segment_descriptor *)gdtr->base;
    size_t num_descriptors =
        gdtr->limit / sizeof(struct mem_segment_descriptor);

    /* copy into kernel-owned memory */
    memcpy((void *)&gdt[1], (const void *)&bootloader_descriptors[1],
           gdtr->limit);

    /* update GDTR */
    gdtr->base = (u64)gdt;
    gdtr->limit = GDT_LIMIT;
    load_gdt(gdtr);

    /* print GDT descriptors */
    debugk("Loaded GDT at address %p with %zu descriptors.\n", (void *)gdt,
           gdtr->limit / sizeof(struct mem_segment_descriptor));
    for (size_t i = 0; i < num_descriptors; ++i) {
        struct mem_segment_descriptor *desc = &gdt[i + 1];
        debugk("GDT[%.3zu]: base=%#.8x, limit=%#.6x, system=%s, executable=%s, "
               "DC=%s, RW=%s, accessed=%s, dpl=%i, present=%s, "
               "available=%s, long_mode=%s, def32=%s, gran=%s\n",
               i + 1, (desc->hibase << 24) | desc->lobase,
               (desc->hilimit << 16) | desc->lolimit,
               bit_set_string(desc->sd_type >> 4),
               bit_set_string((desc->sd_type >> 3) & 1),
               bit_set_string((desc->sd_type >> 2) & 1),
               bit_set_string((desc->sd_type >> 1) & 1),
               bit_set_string(desc->sd_type & 1), desc->dpl,
               bit_set_string(desc->p), bit_set_string(desc->available),
               bit_set_string(desc->long_mode), bit_set_string(desc->def32),
               bit_set_string(desc->gran));
    }
}

#endif
