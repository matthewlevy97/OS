#include <amd64/asm/boot.h>
#include <amd64/asm/segments.h>
#include <amd64/paging.h>

#define GDT_PRESENT                    (1 << 15)
#define GDT_CODE                       (3 << 11)
#define GDT_LONG_MODE                  (1 << 21)

.intel_syntax noprefix

.section .early_boot_structs

.align PAGE_SIZE
.global boot_p4
.global boot_gdt_ptr

/*
    Setup 4-tier paging and memory map first 2MB
*/
boot_p4:
    /* Identity map */
    .quad offset V2P(boot_p3) + (PAGE_PRESENT | PAGE_WRITE)
    .rept 255
        .quad 0
    .endr
    /* Map higher-half [Index: EARLY_BOOT_HIGHER_HALF_INDEX=256] */
    .quad offset V2P(boot_p3) + (PAGE_PRESENT | PAGE_WRITE)
    .rept PAGING_ENTRIES_PER_PT - (EARLY_BOOT_HIGHER_HALF_INDEX - 1) - 2
        .quad 0
    .endr
boot_p3:
    .quad offset V2P(boot_p2) + (PAGE_PRESENT | PAGE_WRITE)
    .rept PAGING_ENTRIES_PER_PT - 1
        .quad 0
    .endr
boot_p2:
    .quad offset V2P(boot_p1) + (PAGE_PRESENT | PAGE_WRITE)
    .rept PAGING_ENTRIES_PER_PT - 1
        .quad 0
    .endr
boot_p1:
    .set i, 0
    .rept PAGING_ENTRIES_PER_PT - 1
        .quad (i << 12) + (PAGE_PRESENT | PAGE_WRITE)
        .set i, (i+1)
    .endr

.section .rodata
.align 16
boot_gdt:
	.long 0, 0, 0, (GDT_PRESENT | GDT_CODE | GDT_LONG_MODE)
boot_gdt_ptr:
	.short (2 * 8) - 1 /* Size of GDT Table - 1 */
	.quad offset boot_gdt