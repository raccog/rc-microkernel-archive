        global x86_sidt
        global x86_lidt
        global idt
        global isr
        global isr_test

        extern isr_handler

        %define IDT_SIZE (16 * 256)

        section .text
        align 8
x86_sidt:
        sidt [rdi]
        ret

        align 8
x86_lidt:
        lidt [rdi]
        ret

        align 8
isr:
        push rax
        push rcx
        push rdx
        push r8
        push r9
        push r10
        push r11
        cld
        call isr_handler
        pop r11
        pop r10
        pop r9
        pop r8
        pop rdx
        pop rcx
        pop rax
        iretq

        ; force divide by zero exception
isr_test:
        mov rax, 10
        mov bl, 0
        div bl
        ret

        section .data
        align 16
idt:
        times IDT_SIZE db 0
