        global x86_64_get_gdt

        section .data
        align 8
gdtr:
        gdtr_limit  dw 0    ; limit
        gdtr_base   dq 0    ; base

        section .text
x86_64_get_gdt:
        sgdt [gdtr]
        mov ax, [gdtr_limit]
        mov rdx, [gdtr_base]
        ret


