# This makefile is inspired by the template at https://github.com/limine-bootloader/limine-barebones

BUILDDIR_BASE ?= Build
ifeq ($(MAKECMDGOALS),test)
BUILDDIR ?= $(BUILDDIR_BASE)/test
else
BUILDDIR ?= $(BUILDDIR_BASE)/prod
endif
CACHEDIR ?= .cache

#
# Variables that cannot be changed by the user
#

override KERNEL_IMAGE := $(BUILDDIR)/rc-microkernel.elf
override INSTALL_DISK := $(BUILDDIR)/image.hdd
override MOUNTDIR := $(CACHEDIR)/img_mount
override LOG_FILE := $(BUILDDIR)/kernel.log

override LIMINEDIR := $(CACHEDIR)/limine

override CC := clang
override CXX := clang++

override DEFAULT_ARCH := x86_64

#
# Variables that can be changed by the user
#

ARCH ?= $(DEFAULT_ARCH)
ARCHDIR := Arch/$(ARCH)

BOOTLOADER ?= Limine
BOOTLOADERDIR ?= Bootloader/$(BOOTLOADER)

include Kernel/Makefile
include Kernel/$(ARCHDIR)/Makefile
include Kernel/$(BOOTLOADERDIR)/Makefile

include RC/Makefile

override KERNEL_H := $(shell find Include/ -type f -name '*.h')

CFLAGS ?= -O2 -g -Wall -Wextra -Wpedantic -IInclude -std=c17 \
	--target=$(ARCH_TRIPLE) -DARCH_$(ARCH)
CPPFLAGS ?=
NASMFLAGS ?= -F dwarf -g -f elf64
LDFLAGS ?=

ifeq ($(MAKECMDGOALS),test)
include Kernel/Tests/Makefile
endif

# Internal C flags that should not be changed by the user
override KERNEL_CFLAGS := 	\
	-IInclude/Kernel		\
	-ffreestanding			\
	-fno-builtin			\
	-fno-pie				\
	-fno-pic				\
	-mabi=sysv				\
	-mcmodel=kernel			\
	-MMD

# Internal linker flags
override KERNEL_LDFLAGS := 		\
	-nostdlib					\
	-static						\
	-Wl,-z,max-page-size=0x1000	\
	-Wl,-T,Kernel/linker.ld

# File globs
override KERNEL_OBJ := $(patsubst %.c,$(BUILDDIR)/%.o,$(KERNEL_SRC))
override KERNEL_OBJ_ASM := $(patsubst %.asm,$(BUILDDIR)/%.o,$(KERNEL_SRC_ASM))

override KERNEL_DEPS := $(KERNEL_SRC:.c=.d) $(KERNEL_SRC_ASM:.asm=.d)
override DEFAULT_H := RC/stdint.h stdbool.h
override DEFAULT_H := $(patsubst %.h,-include %.h,$(DEFAULT_H))

#
# Rules
#

.PHONY: all
all: format $(KERNEL_IMAGE)

$(KERNEL_IMAGE): $(KERNEL_OBJ) $(KERNEL_OBJ_ASM)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(KERNEL_CFLAGS) $(ARCH_CFLAGS) $(LDFLAGS) $(KERNEL_LDFLAGS) -o $@ $^

-include $(KERNEL_DEPS)

$(BUILDDIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(DEFAULT_H) $(CFLAGS) $(KERNEL_CFLAGS) $(ARCH_CFLAGS) -c $< -o $@

$(BUILDDIR)/%.o: %.asm
	@mkdir -p $(@D)
	nasm $(NASMFLAGS) $(KERNEL_NASMFLAGS) -o $@ $<

$(INSTALL_DISK): $(KERNEL_IMAGE) $(LIMINEDIR)
	@mkdir -p $(@D)
	./install_loopback.sh $@ $<

$(LIMINEDIR):
	@mkdir -p $(@D)
	git clone https://github.com/limine-bootloader/limine.git --branch=v3.0-branch-binary --depth=1 $@
	make -C $@

# Save previous log
.PHONY: save-log
save-log:
	if [[ -f $(LOG_FILE) ]]; then mv $(LOG_FILE) $(BUILDDIR)/kernel.old.log; fi

# Output serial to console as well as log
.PHONY: run test
run test: format $(INSTALL_DISK) save-log
	qemu-system-x86_64 \
		-m 256M \
		-hda $(INSTALL_DISK) \
		-chardev stdio,id=char0,logfile=$(LOG_FILE) \
		-serial chardev:char0

# Output serial to only log
.PHONY: run-serial
run-serial: format $(INSTALL_DISK) save-log
	qemu-system-x86_64 \
		-m 256M \
		-hda $(INSTALL_DISK) \
		-serial file:$(LOG_FILE)

.PHONY: format
format: $(KERNEL_SRC) $(KERNEL_H)
	clang-format -i -style=file $^

.PHONY: tidy
tidy: $(KERNEL_SRC) $(KERNEL_H)
	clang-tidy $^ -- $(DEFAULT_H) $(CFLAGS) $(KERNEL_CFLAGS) $(ARCH_CFLAGS)

.PHONY: docs
docs:
	mkdir -p $(BUILDDIR)
	doxygen

.PHONY: docs-open
docs-open: docs
	firefox --new-tab $(BUILDDIR)/doxygen/html/index.html

.PHONY: clean
clean:
	rm -rf $(BUILDDIR_BASE)

.PHONY: nuke
nuke: clean
	rm -rf $(CACHEDIR)
