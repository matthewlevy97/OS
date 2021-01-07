TARGET_ARCH:=x86_64

TARGET:=$(TARGET_ARCH)-elf
CC:=$(TARGET)-gcc
LD:=$(TARGET)-ld
AS:=$(TARGET)-as
AR:=$(TARGET)-ar

CFLAGS += -std=c11 -ffreestanding -nostdlib
CFLAGS += -Wall -Wextra -pedantic
CPPFLAGS +=
LDFLAGS +=
ASFLAGS +=

KERNEL_CFLAGS := $(CFLAGS)
KERNEL_CFLAGS += --sysroot=$(SYSROOT) -isystem=/usr/include -mcmodel=large
KERNEL_CFLAGS += -mno-red-zone -mno-mmx -mno-sse -mno-sse2
KERNEL_CPPFLAGS := $(CPPFLAGS)
KERNEL_LDFLAGS := $(LDFLAGS)
KERNEL_LDFLAGS :=  -n -nostdlib -lgcc
KERNEL_ASFLAGS := $(ASFLAGS)

# Enable Debug symbols?
DEBUG=1
ifdef DEBUG
KERNEL_CFLAGS += -O0 -ggdb -D_KERNEL_DEBUG
KERNEL_ASFLAGS += -ggdb
else
KERNEL_CFLAGS += -O3
endif

# Set the correct kernel architecture
KERNEL_CFLAGS += -DARCH_$(TARGET_ARCH)

COMPILER_PROGRAMS:= \
	CC="$(CC)" \
	LD="$(LD)" \
	AS="$(AS)" \
	AR="$(AR)" \

COMPILE_FLAGS:= \
	$(COMPILER_PROGRAMS) \
	CFLAGS="$(CFLAGS)" \
	CPPFLAGS="$(CPPFLAGS)" \
	LDFLAGS="$(LDFLAGS)" \
	ASFLAGS="$(ASFLAGS)"

# Flags for compiling the kernel
KERNEL_COMPILE_FLAGS:= \
	$(COMPILER_PROGRAMS) \
	KERNEL_CFLAGS="$(KERNEL_CFLAGS)" \
	KERNEL_CPPFLAGS="$(KERNEL_CPPFLAGS)" \
	KERNEL_LDFLAGS="$(KERNEL_LDFLAGS)" \
	KERNEL_ASFLAGS="$(KERNEL_ASFLAGS)"

.PHONY: all sysroot install-gcc-headers clean mkiso kernel libc

all: sysroot install-gcc-headers libc kernel

sysroot:
	mkdir -p ${SYSROOT}

install-gcc-headers: sysroot
	@echo "Installing GCC header files"
	mkdir -p "${SYSROOT}/usr/include/gcc"
	@for hfile in iso646.h stdarg.h stdbool.h stddef.h; do			\
		hpath=$$(find $$GCC_INCLUDE_PATH -name $$hfile);		\
		if [ ! -z $$hpath ]; then					\
			cp "$$hpath" "${SYSROOT}/usr/include/gcc/$$hfile";	\
		else								\
			echo "$$hfile not found";				\
			exit 1;							\
		fi								\
	done

# Build libc and libk
libc: sysroot
	$(MAKE) -C libc install $(COMPILE_FLAGS) $(KERNEL_COMPILE_FLAGS)

# Build the kernel
kernel: sysroot
	$(MAKE) -C kernel install $(KERNEL_COMPILE_FLAGS)

mkiso: all
	@mkdir -p ${SYSROOT}/boot/grub
	echo 'set timeout=1' > ${SYSROOT}/boot/grub/grub.cfg
	echo 'set default=0' >> ${SYSROOT}/boot/grub/grub.cfg
	echo 'menuentry "os" {' >> ${SYSROOT}/boot/grub/grub.cfg
	echo '	multiboot2 /boot/kernel' >> ${SYSROOT}/boot/grub/grub.cfg
	echo '	boot' >> ${SYSROOT}/boot/grub/grub.cfg
	echo '}' >> ${SYSROOT}/boot/grub/grub.cfg
	grub-mkrescue -o ${ISO} ${SYSROOT} > /dev/null

clean:
	$(MAKE) -C libc clean
	$(MAKE) -C kernel clean
	rm -rf $(SYSROOT)
	rm -rf $(ISO)
