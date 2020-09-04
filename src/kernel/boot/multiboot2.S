.intel_syntax noprefix

#include <boot/multiboot2.h>
.section .multiboot
.align 0x8

.multiboot_header_start:
	.long MULTIBOOT2_MAGIC
	.long MULTIBOOT2_ARCH
	.long MULTIBOOT2_LENGTH
	.long MULTIBOOT2_CHECKSUM

	// Termination tag
	.short 0 // type
	.short 0 // flags
	.long  8 // size

.multiboot_header_end: