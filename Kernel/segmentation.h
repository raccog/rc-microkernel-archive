#pragma once

/**
 * An x86_64 segment descriptor to be used in a GDT or LDT.
 */
struct x86_64_segment_descriptor {
    u16 limit0;
    u16 base0;
    u16 base1;
    u16 access;
    u8 limit1;
    u8 flags;
    u16 base2;
};

/**
 * A register pointing to an x86_64 GDT.
 */
struct x86_64_gdt_register {
    u16 limit;
    struct x86_64_segment_descriptor *base;
} __attribute__((packed));

/**
 * Print all segments in the currently set x86_64 GDT.
 */
void x86_64_print_gdt();

/**
 * Returns the address of the current x86_64 GDT.
 *
 * @return A register pointing to the current GDT.
 */
extern struct x86_64_gdt_register x86_64_get_gdt();
