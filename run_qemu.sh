#!/bin/bash -e

source $(dirname "$0")/setup_env.sh

seriallog="$(dirname "$0")/serial.log"

if [ ! -f "${ISO}" ]; then
    echo "[-] Must build ISO image before starting qemu!"
    exit 0
fi

# Run qemu
qemu-system-x86_64.exe -cdrom kernel.iso -serial file:${seriallog} -no-reboot $@