/**
 * Declarations for GDT/IDT.
 *
 * Most structs are taken from OpenBSD `sys/arch/amd64/include/segments.h`.
 */

#ifdef ARCH_x86_64

#pragma once

/**
 * The descriptor used to load and store GDT/IDT.
 */
struct region_descriptor {
    u16 limit; /* segment limit */
    u64 base;  /* segment base address */
} __attribute__((packed));

#endif
