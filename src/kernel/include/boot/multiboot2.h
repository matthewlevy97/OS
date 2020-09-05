#pragma once

#ifdef __ASSEMBLER__

#define MULTIBOOT2_MAGIC              0xE85250D6 // Request
#define MULTIBOOT2_ARCH               0
#define MULTIBOOT2_LENGTH             (.multiboot_header_end - .multiboot_header_start)
#define MULTIBOOT2_CHECKSUM           -(MULTIBOOT2_MAGIC + MULTIBOOT2_ARCH + MULTIBOOT2_LENGTH)

#else

#include <stdint.h>

#define MULTIBOOT2_MAGIC               0x36D76289 // Reply

typedef uint32_t multiboot_magic_t;
typedef void* multiboot_data_t;

#endif