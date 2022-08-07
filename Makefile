# This makefile is based on the template at https://github.com/limine-bootloader/limine-barebones

BUILD ?= Build
CACHE ?= .cache
override KERNEL := $(BUILD)/rc-microkernel.elf
override IMAGE := $(BUILD)/image.hdd
override LIMINE := $(CACHE)/limine
override MOUNT := $(CACHE)/img_mount

override LOG_FILE := $(BUILD)/kernel.log

# Convenience macro to reliably declare overridable command variables.
define DEFAULT_VAR =
	ifeq ($(origin $1), default)
		override $(1) := $(2)
	endif
	ifeq ($(origin $1), undefined)
		override $(1) := $(2)
	endif
endef

# Use clang as default compiler
$(eval $(call DEFAULT_VAR,CC,clang))
$(eval $(call DEFAULT_VAR,CXX,clang++))

CFLAGS ?=
NASMFLAGS ?= -F dwarf -g
LDFLAGS ?=
CXXFLAGS ?= -O2 -g -Wall -Wextra -Wpedantic

# Internal C++ flags
override INTERNALCXXFLAGS := 	\
	-I.					 	\
	-ffreestanding			\
	-fno-builtin			\
	-fno-exceptions			\
	-fno-rtti				\
	-fno-stack-protector	\
	-fno-stack-check		\
	-fno-pie				\
	-fno-pic				\
	-m64					\
	--target=x86_64-pc-none-elf \
	-march=x86-64			\
	-mabi=sysv				\
	-mno-80387				\
	-mno-mmx				\
	-mno-sse				\
	-mno-sse2				\
	-mno-red-zone			\
	-mcmodel=kernel			\
	-MMD					\
	-std=c++20

# Internal linker flags
override INTERNALLDFLAGS := 	\
	-nostdlib					\
	-static						\
	-Wl,-z,max-page-size=0x1000	\
	-Wl,-T,Kernel/linker.ld

# Internal NASM flags
override INTERNALNASMFLAGS :=	\
	-f elf64

# File globs
override KERNEL_SRC := $(shell find {Kernel/,RC/} -type f -name '*.cpp')
override KERNEL_SRC_ASM := $(shell find Kernel/ -type f -name '*.asm')
override KERNEL_OBJ := $(patsubst %.cpp,$(BUILD)/%.o,$(KERNEL_SRC))
override KERNEL_OBJ_ASM := $(patsubst %.asm,$(BUILD)/%.o,$(KERNEL_SRC_ASM))
override KERNEL_H := $(shell find {Kernel/,RC/} -type f -name '*.h')

override STDINT_H := RC/Stdint.h

#
# Rules
#

.PHONY: all
all: format $(KERNEL)

$(KERNEL): $(KERNEL_OBJ) $(KERNEL_OBJ_ASM)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INTERNALCXXFLAGS) $(LDFLAGS) $(INTERNALLDFLAGS) -o $@ $^

$(BUILD)/%.o: %.cpp $(KERNEL_H)
	@mkdir -p $(@D)
	$(CXX) -include $(STDINT_H) $(CXXFLAGS) $(INTERNALCXXFLAGS) -c $< -o $@

$(BUILD)/%.o: %.asm
	@mkdir -p $(@D)
	nasm -g -f elf64 -o $@ $<

$(IMAGE): $(KERNEL) $(LIMINE)
	@mkdir -p $(@D)
	dd if=/dev/zero bs=1M count=0 seek=64 of=$@
	parted -s $@ mklabel gpt
	parted -s $@ mkpart ESP fat32 2048s 100%
	parted -s $@ set 1 esp on
	$(LIMINE)/limine-deploy $@
	USED_LOOPBACK=$$(sudo losetup -Pf --show $@) ;\
	sudo mkfs.fat -F 32 $${USED_LOOPBACK}p1 ;\
	mkdir -p $(MOUNT) ;\
	sudo mount $${USED_LOOPBACK}p1 $(MOUNT) ;\
	sudo mkdir -p $(MOUNT)/EFI/BOOT ;\
	sudo cp -v $(KERNEL) limine.cfg $(LIMINE)/limine.sys $(MOUNT) ;\
	sudo cp -v $(LIMINE)/BOOTX64.EFI $(MOUNT)/EFI/BOOT ;\
	sync ;\
	sudo umount $(MOUNT) ;\
	sudo losetup -d $$USED_LOOPBACK

$(LIMINE):
	@mkdir -p $(@D)
	git clone https://github.com/limine-bootloader/limine.git --branch=v3.0-branch-binary --depth=1 $@
	make -C $@

# Save previous log
.PHONY: save-log
save-log:
	if [[ -f $(LOG_FILE) ]]; then mv $(LOG_FILE) $(BUILD)/kernel.old.log; fi

# Output serial to console as well as log
.PHONY: run
run: format $(IMAGE) save-log
	qemu-system-x86_64 \
		-hda $(IMAGE) \
		-chardev stdio,id=char0,logfile=$(LOG_FILE) \
		-serial chardev:char0

# Output serial to only log
.PHONY: run-serial
run-serial: format $(IMAGE) save-log
	qemu-system-x86_64 \
		-hda $(IMAGE) \
		-serial file:$(LOG_FILE)

.PHONY: format
format: $(KERNEL_SRC) $(KERNEL_H)
	clang-format -i -style=file $^

.PHONY: docs
docs:
	doxygen

.PHONY: docs-open
docs-open: docs
	firefox --new-tab $(BUILD)/doxygen/html/index.html

.PHONY: clean
clean:
	rm -rf $(BUILD)

.PHONY: nuke
nuke: clean
	rm -rf $(CACHE)
