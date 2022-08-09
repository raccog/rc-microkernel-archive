/** \file
 * Definitions related to system memory segmentation.
 */

#pragma once

/**
 * A segment descriptor to be used in a GDT or LDT.
 */
struct segment_descriptor {
    u16 limit0;
    u16 base0;
    u16 base1;
    u16 access;
    u8 limit1;
    u8 flags;
    u16 base2;
};

/**
 * A register pointing to a GDT.
 */
struct gdt_register {
    u16 limit;
    struct segment_descriptor *base;
}; // __attribute__((packed));

/**
 * Print all segments in the currently set GDT.
 */
void kprint_gdt();

/**
 * Returns the address of the current GDT.
 */
struct gdt_register kget_gdt();
