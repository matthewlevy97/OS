LIBC_BIN := libc.o
LIBK_BIN := libk.o

LIBC_CFLAGS := $(CFLAGS)
LIBC_CPPFLAGS := $(CPPFLAGS) -Iinclude/

LIBK_CFLAGS := $(KERNEL_CFLAGS) -D__IS_LIBK
LIBK_CPPFLAGS := $(KERNEL_CPPFLAGS) -Iinclude/

LIBC_OBJS :=
include string/make.config
include structures/make.config
LIBK_OBJS := $(LIBC_OBJS:.o=.libk.o)

.PHONY: all install-binary install-headers clean
.SUFFIXES: .o .libk.o .c .S

all: $(LIBK_BIN) $(LIBC_BIN)

install: all install-binary install-headers

install-binary:
	@mkdir -p $(SYSROOT)/usr/bin
	@cp -a $(LIBK_BIN) $(SYSROOT)/usr/bin
	@cp -a $(LIBC_BIN) $(SYSROOT)/usr/bin

install-headers:
	@mkdir -p $(SYSROOT)/usr/include/libc
	@cp -a include/* $(SYSROOT)/usr/include/libc

$(LIBK_BIN): $(LIBK_OBJS)
	$(AR) rcs $@ $^

$(LIBC_BIN): $(LIBC_OBJS)
	$(AR) rcs $@ $^

.c.o:
	$(CC) -c $< -o $@ $(LIBC_CFLAGS) $(LIBC_CPPFLAGS)

.S.o:
	$(CC) -c $< -o $@ $(LIBC_CFLAGS) $(LIBC_CPPFLAGS)

.c.libk.o:
	$(CC) -c $< -o $@ $(LIBK_CFLAGS) $(LIBK_CPPFLAGS)

.S.libk.o:
	$(CC) -c $< -o $@ $(LIBK_CFLAGS) $(LIBK_CPPFLAGS)

clean:
	rm -f $(LIBC_BIN) $(LIBK_BIN)
	rm -f $(LIBC_OBJS)
	rm -f $(LIBK_OBJS)