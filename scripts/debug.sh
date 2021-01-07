#!/bin/bash -e

source $(dirname "$0")/../setup_env.sh

seriallog="$(dirname "$0")/../serial.log"
gdbinit=$(dirname "$0")/../toolchain/gdbinit
kernel_bin_path=$(dirname "$0")/../sysroot/boot/kernel

make -C src mkiso

# Run qemu
qemu-system-x86_64.exe -s -S \
    -cdrom kernel.iso -D log.log -serial file:${seriallog} \
    -d int -no-reboot &
gdb -q -x ${gdbinit} -se ${kernel_bin_path}
