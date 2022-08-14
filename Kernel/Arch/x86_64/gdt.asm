        global get_gdt
        global load_gdt
        global gdt

        section .data
gdtr:
        align 4
        gdtr_limit  dw 0    ; limit
        align 8
        gdtr_base   dq 0    ; base

        section .text
get_gdt:
        sgdt [gdtr]
        mov rax, gdtr
        ret

load_gdt:
        lgdt [rdi]
        ret

