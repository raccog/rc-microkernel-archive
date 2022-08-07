OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)

ENTRY(_start)

PHDRS
{
    text    PT_LOAD     FLAGS((1 << 0) | (1 << 2)) ; /* execute + read */
    rodata  PT_LOAD     FLAGS((1 << 2)) ;            /* read only */
    data    PT_LOAD     FLAGS((1 << 1) | (1 << 2)) ; /* write + read */
}

SECTIONS
{
    /* higher half kernel */
    . = 0xffffffff80000000;

    .text : {
        *(.text .text.*)
    } :text

    /* move to next memory page for rodata */
    . += CONSTANT(MAXPAGESIZE);

    .rodata : {
        *(.rodata .rodata.*)
    } :rodata

    /* move to next memory page for data */
    . += CONSTANT(MAXPAGESIZE);

    .data : {
        *(.data .data.*)
    } :data

    .bss : {
        *(COMMON)
        *(.bss .bss.*)
    } :data

    /* Discard notes because they cause a kernel crash */
    /DISCARD/ : {
        *(.note .note.*)
    }
}
