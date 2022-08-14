/**
 * GDT declarations.
 *
 * Most structs are taken from OpenBSD `sys/arch/amd64/include/segments.h`.
 */

#ifdef ARCH_x86_64

#pragma once

/**
 * A descriptor for a memory segment in the GDT.
 */
struct mem_segment_descriptor {
    unsigned int lolimit : 16;  /* segment extent (lsb) */
    unsigned int lobase : 24;   /* segment base address (lsb) */
    unsigned int sd_type : 5;   /* segment type */
    unsigned int dpl : 2;       /* segment descriptor priority level */
    unsigned int p : 1;         /* segment descriptor present */
    unsigned int hilimit : 4;   /* segment extent (msb) */
    unsigned int available : 1; /* available */
    unsigned int long_mode : 1; /* long mode */
    unsigned int def32 : 1;     /* default 32 vs 16 bit size */
    unsigned int gran : 1;      /* limit granularity (byte/page) */
    unsigned int hibase : 8;    /* segment base address (msb) */
} __attribute__((packed));

/**
 * The descriptor used to load and store GDT/IDT.
 */
struct region_descriptor {
    u16 limit; /* segment limit */
    u64 base;  /* segment base address */
} __attribute__((packed));

/**
 * Initialize the GDT.
 */
void init_gdt();

/**
 * Returns the currently loaded GDT's region descriptor.
 */
extern struct region_descriptor *get_gdt();

#endif
