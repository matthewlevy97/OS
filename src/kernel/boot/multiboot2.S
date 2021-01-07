.intel_syntax noprefix

#include <boot/multiboot2.h>
.section .multiboot
.align 0x8

.multiboot_header_start:
	.long MULTIBOOT2_MAGIC
	.long MULTIBOOT2_ARCH
	.long MULTIBOOT2_LENGTH
	.long MULTIBOOT2_CHECKSUM

	// Modules must be page aligned
	.short 6
	.short 0
	.long  8

	// Information request
.info_request_start:
	.short 1 // type
	.short 0 // flags
	.long  (.info_request_end - .info_request_start) // size
	.long  1 // Boot command line
	.long  3 // Modules
	.long  6 // Memory Map
	.long  9 // ELF-Symbols
.info_request_end:

	// Termination tag
	.short 0 // type
	.short 0 // flags
	.long  8 // size

.multiboot_header_end: