/**
 * IDT declarations.
 */
#ifdef ARCH_x86_64

#pragma once

#include "segment.h"

/**
 * A descriptor for an interrupt gate in the IDT.
 */
struct gate_descriptor {
    u16 loofset;          /* gate procedure offset (lsb) */
    u16 segment_selector; /* code segment selector for gate procedure */
    u16 ist : 3;          /* interrupt stack table */
    u16 _reserved0 : 5;   /* reserved zero bits */
    u16 gtype : 4;        /* gate type */
    u16 _reserved1 : 1;   /* reserved zero bits */
    u16 dpl : 2;          /* gate descriptor privilege level */
    u16 present : 1;      /* gate descriptor present */
    u64 hioffset : 48;    /* gate procedure offset (msb) */
    u32 _reserved2;       /* reserved zero bits */
} __attribute__((packed));

/**
 * Initialize the IDT.
 */
void init_idt();

/**
 * Generic interrupt service routine.
 */
extern void isr();

/**
 * Sets `idtr` to the current IDT register.
 *
 * Note: The memory pointed to by `idtr` must be at least 10 bytes in length.
 *
 * @param idtr A location in memory to store the current IDTR.
 */
extern void x86_sidt(struct region_descriptor *idtr);

/**
 * Loads a region descriptor (IDTR) as the current IDT.
 *
 * @param idtr The IDTR to load.
 */
extern void x86_lidt(const struct region_descriptor *idtr);

#endif
