#!/bin/bash -e

source $(dirname "$0")/setup_env.sh

seriallog="$(dirname "$0")/serial.log"

make -C src mkiso

# Run qemu
qemu-system-x86_64.exe -cdrom kernel.iso \
    -serial file:${seriallog} -no-reboot $@