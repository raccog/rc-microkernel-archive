/**
 * GDT declarations.
 *
 * Most structs are taken from OpenBSD `sys/arch/amd64/include/segments.h`.
 */

#ifdef ARCH_x86_64

#pragma once

#include "segment.h"

/**
 * A descriptor for a memory segment in the GDT.
 */
struct mem_segment_descriptor {
    u16 lolimit;      /* segment extent (lsb) */
    u32 lobase : 24;  /* segment base address (lsb) */
    u32 sd_type : 5;  /* segment type */
    u32 dpl : 2;      /* segment descriptor privilege level */
    u32 present : 1;  /* segment descriptor present */
    u8 hilimit : 4;   /* segment extent (msb) */
    u8 available : 1; /* available */
    u8 long_mode : 1; /* long mode */
    u8 def32 : 1;     /* default 32 vs 16 bit size */
    u8 gran : 1;      /* limit granularity (byte/page) */
    u8 hibase;        /* segment base address (msb) */
} __attribute__((packed));

/**
 * Initialize the GDT.
 */
void init_gdt();

/**
 * Sets `gdtr` to the current GDT register.
 *
 * Note: The memory pointed to by `gdtr` must be at least 10 bytes in length.
 *
 * @param gdtr A location in memory to store the current GDTR.
 */
extern void x86_sgdt(struct region_descriptor *gdtr);

/**
 * Loads a region descriptor (GDTR) as the current GDT.
 *
 * @param gdtr The GDTR to load.
 */
extern void x86_lgdt(const struct region_descriptor *gdtr);

#endif
