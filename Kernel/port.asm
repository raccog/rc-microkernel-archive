        global outb
        global inb

        section .text
outb:
        mov dx, di
        mov ax, si
        out dx, al
        ret

inb:
        mov dx, di
        in al, dx
        ret
