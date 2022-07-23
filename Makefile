# This makefile is based on the template at https://github.com/limine-bootloader/limine-barebones

BUILD ?= build
CACHE ?= .cache
override KERNEL := $(BUILD)/rc-microkernel.elf
override IMAGE := $(BUILD)/image.hdd
override LIMINE := $(CACHE)/limine
override MOUNT := $(CACHE)/img_mount

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

CFLAGS ?= -O2 -g -Wall -Wextra -Wpedantic
NASMFLAGS ?= -F dwarf -g
LDFLAGS ?=
CPPFLAGS ?= -O2 -g -Wall -Wextra -Wpedantic

# Internal common flags
override INTERNALFLAGS := 	\
	-I.					 	\
	-ffreestanding			\
	-fno-builtin			\
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
	-MMD


# Internal C flags
override INTERNALCFLAGS := 	\
	$(INTERNALFLAGS)		\
	-std=c11				\

# Internal CXX flags
override INTERNALCXXFLAGS :=\
	$(INTERNALFLAGS)		\
	-std=c++17				\
	-fno-exceptions			\
	-fno-rtti				\

# Internal linker flags
override INTERNALLDFLAGS := 	\
	-nostdlib					\
	-static						\
	-Wl,-z,max-page-size=0x1000	\
	-Wl,-T,kernel/linker.ld

# Internal NASM flags
override INTERNALNASMFLAGS :=	\
	-f elf64

# File globs
override KERNEL_SRC := $(shell find kernel/ -type f -name '*.cpp')
override KERNEL_OBJ := $(patsubst %.cpp,build/%.o,$(KERNEL_SRC))
override HEADER_DEPS := $(patsubst %.cpp,build/%.d,$(KERNEL_SRC))

#
# Rules
#

.PHONY: all
all: $(KERNEL)

$(KERNEL): $(KERNEL_OBJ)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INTERNALCXXFLAGS) $(KERNEL_OBJ) $(LDFLAGS) $(INTERNALLDFLAGS) -o $@

# Include header dependencies
-include $(HEADER_DEPS)

$(KERNEL_OBJ): $(KERNEL_SRC)
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CFLAGS) $(INTERNALCXXFLAGS) -c $< -o $@

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

.PHONY: run
run: $(IMAGE)
	qemu-system-x86_64 -hda $(IMAGE)

.PHONY: clean
clean:
	rm -rf $(BUILD)

.PHONY: nuke
nuke: clean
	rm -rf $(CACHE)
