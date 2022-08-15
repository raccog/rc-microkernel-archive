override ARCH_TRIPLE := x86_64-elf
override ARCH_CFLAGS :=      \
	-DBOOTLOADER_LIMINE 	\
	-fno-stack-protector	\
	-fno-stack-check		\
	-m64					\
	-mno-80387				\
	-mno-mmx				\
	-mno-sse				\
	-mno-sse2				\
	-mno-red-zone			\
	-march=x86-64

KERNEL_SRC += 						\
	Kernel/Arch/x86_64/gdt.c		\
	Kernel/Arch/x86_64/idt.c		\
	Kernel/Arch/x86_64/init.c		\
	Kernel/Arch/x86_64/serial.c

KERNEL_SRC_ASM +=					\
	Kernel/Arch/x86_64/gdt.asm		\
	Kernel/Arch/x86_64/idt.asm		\
	Kernel/Arch/x86_64/ports.asm
