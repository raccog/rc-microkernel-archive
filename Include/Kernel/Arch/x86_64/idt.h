/**
 * IDT declarations.
 */
#ifdef ARCH_x86_64

#pragma once

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
// void init_idt();

#endif
