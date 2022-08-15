#ifdef ARCH_x86_64

#include "Arch/x86_64/idt.h"
#include "Arch/x86_64/segment.h"
#include "RC/string.h"

#include "printk.h"

/* 256 IDT descriptors */
#define IDT_LEN 256
/* size of IDT in bytes */
#define IDT_SIZE (sizeof(struct gate_descriptor) * IDT_LEN)

/* IDTR memory location */
static struct region_descriptor idtr;

/* IDT memory location */
extern struct gate_descriptor idt[IDT_LEN] __attribute__((aligned(0x10)));

#define KERNEL_CODE_SEGMENT_INDEX 5
#define GATE_TYPE_INTERRUPT 14

/* temporary test for isr */
extern void isr_test();

void init_idt() {
    struct gate_descriptor *divide_gate = &idt[0];
    divide_gate->loofset = (u16)(((u64)&isr) & 0xffff);
    divide_gate->segment_selector = KERNEL_CODE_SEGMENT_INDEX << 3;
    divide_gate->ist = 0;
    divide_gate->gtype = GATE_TYPE_INTERRUPT;
    divide_gate->dpl = 0;
    divide_gate->present = 1;
    divide_gate->hioffset = ((((u64)&isr) >> 16) & 0xffffffffffff);

    idtr.base = (u64)idt;
    idtr.limit = IDT_SIZE;

    x86_lidt(&idtr);
    debugk("Loaded IDTR at %p\n", (void *)&idtr);
    debugk("IDTR limit=%hu, base=%#lx\n", idtr.limit, idtr.base);

    isr_test();
}

void isr_handler() {
    debugk("Divide error???\n");
    __asm__ volatile("hlt");
}

#endif
