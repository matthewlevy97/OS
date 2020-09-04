#!/bin/bash -e

source $(dirname "$0")/../setup_env.sh

binutils=binutils-2.29
gcc=gcc-7.2.0

function build_binutils()
{
    if [ ! -d "${PREFIX}/dl/${binutils}" ]; then
        cd $PREFIX/dl
        wget "http://ftp.gnu.org/gnu/binutils/${binutils}.tar.gz"
        echo "[*] Extracting ${binutils}.tar.gz"
        tar -xf "${binutils}.tar.gz"
    else
        echo "[*] Binutils (${binutils}) already extracted!"
    fi

    if [ ! -f "${PREFIX}/bin/${TARGET}-ld" ]; then
        mkdir -p ${PREFIX}/build/binutils && cd ${PREFIX}/build/binutils
        ${PREFIX}/dl/${binutils}/configure \
            --target="${TARGET}"           \
            --prefix="$PREFIX"             \
            --with-sysroot                 \
            --disable-nls                  \
            --disable-werror
        make -j 4
        make install
    else
        echo "[*] Binutils [$(${PREFIX}/bin/${TARGET}-ld -v)] already compiled and installed!"
    fi
}

function build_gcc()
{
    # The $PREFIX/bin dir _must_ be in the PATH. We did that above.
    if [[ ! $(which -- "x86_64-elf-as") ]]; then
        echo "${TARGET}-as is not in the PATH"
        exit 1
    fi
    
    if [ ! -d "${PREFIX}/dl/${gcc}" ]; then
        cd $PREFIX/dl
        wget http://ftp.gnu.org/gnu/gcc/${gcc}/${gcc}.tar.gz
        echo "[*] Extracting ${gcc}.tar.gz"
        tar -xf ${gcc}.tar.gz

        # Setup to compile version with no-red-zone
        mkdir -p ${gcc}/gcc/config/i386/
        echo "MULTILIB_OPTIONS += mno-red-zone" > ${gcc}/gcc/config/i386/t-x86_64-elf
        echo "MULTILIB_DIRNAMES += no-red-zone" >> ${gcc}/gcc/config/i386/t-x86_64-elf
        sed -i 's|x86_64-\*-elf\*)|x86_64-\*-elf\*)\n\ttmake_file="${tmake_file} i386/t-x86_64-elf"|g' ${gcc}/gcc/config.gcc
    else
        echo "[*] GCC (${gcc}) already extracted!"
    fi

    if [ ! -f "${PREFIX}/bin/${TARGET}-gcc" ]; then
        mkdir -p ${PREFIX}/build/gcc && cd ${PREFIX}/build/gcc
        ${PREFIX}/dl/${gcc}/configure \
            --target="${TARGET}"      \
            --prefix="$PREFIX"        \
            --disable-nls             \
            --enable-languages=c      \
            --without-headers

        make all-gcc all-target-libgcc -j 4
        make install-gcc
        make install-target-libgcc
    else
        echo "[*] GCC [$(${PREFIX}/bin/${TARGET}-gcc --version | head -n1)] already compiled and installed!"
    fi
}

# Setup the environment
echo "[*] Setting up build environment"
mkdir -p $PREFIX/dl
mkdir -p $PREFIX/build
mkdir -p $PREFIX/bin

# Install dependencies
echo "[*] Installing dependencies"
sudo apt-get update -y
sudo apt-get install -y grub2-common xorriso qemu-system-x86
sudo apt-get install -y python3 build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo
#sudo apt-get install -y gdb valgrind

# Run the build functions
echo "[*] Building - Binutils"
build_binutils
echo "[*] Building - GCC"
build_gcc