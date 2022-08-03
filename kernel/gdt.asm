        global kget_gdt

        section .data
        align 8
gdtr:
        gdtr_limit  dw 2    ; limit
        gdtr_base   dq 0    ; base

        section .text
kget_gdt:
        sgdt [gdtr]
        mov ax, [gdtr_limit]
        mov rdx, [gdtr_base]
        ret


