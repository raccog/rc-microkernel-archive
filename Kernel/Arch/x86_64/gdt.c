#ifdef ARCH_x86_64

#include "Arch/x86_64/gdt.h"
#include "Arch/x86_64/segment.h"
#include "RC/string.h"

/* 256 GDT descriptors + 1 null descriptor */
#define GDT_LEN (256 + 1)
/* size of GDT in bytes */
#define GDT_SIZE (sizeof(struct mem_segment_descriptor) * GDT_LEN)
/* GDT limit (size not including null descriptor) */
#define GDT_LIMIT (GDT_SIZE - sizeof(struct mem_segment_descriptor))

/* GDTR memory location */
static struct region_descriptor gdtr;

/* GDT memory location */
static struct mem_segment_descriptor gdt[GDT_LEN]
    __attribute__((aligned(0x10)));

void init_gdt() {
    /* get GDT from system */
    x86_sgdt(&gdtr);
    struct mem_segment_descriptor *old_descriptors =
        (struct mem_segment_descriptor *)gdtr.base;

    /* copy into kernel-owned memory */
    memcpy((void *)&gdt[1], (const void *)&old_descriptors[1], gdtr.limit);

    /* update GDTR */
    gdtr.base = (u64)gdt;
    gdtr.limit = GDT_LIMIT;
    x86_lgdt(&gdtr);
}

#endif
