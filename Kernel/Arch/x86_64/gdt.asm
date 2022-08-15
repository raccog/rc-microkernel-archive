        global x86_sgdt
        global x86_lgdt

        section .text
x86_sgdt:
        sgdt [rdi]
        ret

x86_lgdt:
        lgdt [rdi]
        ret

