LINKER_SCRIPT=linker.ld
LIBC_DIRECTORY=../libc
KERNEL_BIN="$(KERNEL_NAME).bin"

CFLAGS := $(KERNEL_CFLAGS)
CPPFLAGS := $(KERNEL_CPPFLAGS) -Iinclude/ -I$(LIBC_DIRECTORY)/include/
LDFLAGS := $(KERNEL_LDFLAGS) -T $(LINKER_SCRIPT)
ASFLAGS := $(KERNEL_ASFLAGS)

KERNEL_OBJS=
include amd64/make.config
include boot/make.config

.PHONY: all install install-binary install-headers clean
.SUFFIXES: .o .c .S .S.py

all: $(KERNEL_BIN)

install: all install-binary install-headers

install-binary:
	@echo "Copying kernel to $(SYSROOT)/boot/$(KERNEL_NAME)"
	@mkdir -p $(SYSROOT)/boot
	@cp -a $(KERNEL_BIN) $(SYSROOT)/boot/$(KERNEL_NAME)

install-headers:
	@mkdir -p $(SYSROOT)/usr/include/$(KERNEL_NAME)
	@cp -a include/* $(SYSROOT)/usr/include/$(KERNEL_NAME)

$(KERNEL_BIN): $(KERNEL_OBJS)
	$(LINK.c) $^ -o $@

%.o: %.S.py
	python3 $^ | $(COMPILE.S) $(DEPFLAGS) -x assembler-with-cpp - -o $@

clean:
	rm -f $(KERNEL_BIN)
	rm -f $(KERNEL_OBJS)