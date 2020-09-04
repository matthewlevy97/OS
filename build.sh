#!/bin/bash -e

source $(dirname "$0")/setup_env.sh

if [[ ! -f "${PREFIX}/bin/${TARGET}-ld" || ! -f "${PREFIX}/bin/${TARGET}-gcc" ]]; then
    echo "[*] Need to setup toolchain first!"
    exit 0
fi

make -C src mkiso