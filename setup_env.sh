#!/bin/bash

export KERNEL_NAME="kernel"

export PREFIX="/mnt/d/opt/cross"
export PATH="$PREFIX/bin:$PATH"
export TARGET=x86_64-elf
export CC=${TARGET}-gcc
export LD=${TARGET}-ld
export AS=${TARGET}-as
export AR=${TARGET}-ar
export SYSROOT=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/sysroot/
export ISO=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/"${KERNEL_NAME}.iso"