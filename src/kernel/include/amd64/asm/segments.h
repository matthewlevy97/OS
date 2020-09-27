#pragma once

/**
 * Shamelessly borrowed from Linux codebase:
 * 	https://github.com/torvalds/linux/blob/ab851d49f6bfc781edd8bd44c72ec1e49211670b/arch/x86/include/asm/segment.h
 */

#define GDT_ENTRY_KERNEL32_CS       1
#define GDT_ENTRY_KERNEL_CS         2
#define GDT_ENTRY_KERNEL_DS         3

/*
 * We cannot use the same code segment descriptor for user and kernel mode,
 * not even in long flat mode, because of different DPL.
 *
 * GDT layout to get 64-bit SYSCALL/SYSRET support right. SYSRET hardcodes
 * selectors:
 *
 *   if returning to 32-bit userspace: cs = STAR.SYSRET_CS,
 *   if returning to 64-bit userspace: cs = STAR.SYSRET_CS+16,
 *
 * ss = STAR.SYSRET_CS+8 (in either case)
 *
 * thus USER_DS should be between 32-bit and 64-bit code selectors:
 */
#define GDT_ENTRY_DEFAULT_USER32_CS 4
#define GDT_ENTRY_DEFAULT_USER_DS   5
#define GDT_ENTRY_DEFAULT_USER_CS   6

// Needs 2 slots
#define GDT_ENTRY_TSS               7

#define GDT_ENTRIES                 9

#define __KERNEL32_CS               (GDT_ENTRY_KERNEL32_CS*8)
#define __KERNEL_CS                 (GDT_ENTRY_KERNEL_CS*8)
#define __KERNEL_DS                 (GDT_ENTRY_KERNEL_DS*8)
#define __USER32_CS                 (GDT_ENTRY_DEFAULT_USER32_CS*8 + 3)
#define __USER_DS                   (GDT_ENTRY_DEFAULT_USER_DS*8 + 3)
#define __USER32_DS                 __USER_DS
#define __USER_CS                   (GDT_ENTRY_DEFAULT_USER_CS*8 + 3)
#define __TSS_SELECTOR              (GDT_ENTRY_TSS*8)

#define GDT_SIZE                    (GDT_ENTRIES*8)

#define SEGMENT_WRITABLE               (1 << 1)
#define SEGMENT_CODE                   (1 << 4 | 0xA)
#define SEGMENT_DATA                   (1 << 4 | 0x2)
#define SEGMENT_USER                   (3 << 5)
#define SEGMENT_PRESENT                (1 << 7)
#define SEGMENT_AVAILABLE              (1 << 12)
#define SEGMENT_LONG_MODE              (1 << 13)
#define SEGMENT_DB                     (1 << 14)
#define SEGMENT_GRANULARITY            (1 << 15)

#define SEGMENT_TSS         (0x9)

#ifndef __ASSEMBLER__

#include <amd64/paging.h>
#include <common.h>
#include <stdint.h>

/**
 * type - segment type
 * s    - descriptor type (0:system, 1:code/data)
 * dpl  - desc priv level
 * p    - segment present
 * avl  - available for use by sys software
 * l    - 64-bit segment (IA-32e mode only)
 * db   - default operation size (0:16bit, 1:32bit)
 * g    - granularity
 */
struct segment_descriptor {
	union {
		struct {
			uint16_t limit0;
			uint16_t base0;
			uint16_t base1 : 8, type : 4, s : 1, dpl : 2, p : 1;
			uint16_t limit1 : 4, avl : 1, l : 1, db : 1, g : 1, base2 : 8;
		};
		uint64_t descriptor;
	};
} __packed;
struct segment_xdescriptor {
	struct segment_descriptor low;
	struct {
		uint32_t base3;
		uint32_t _reserved;
	};
} __packed;

#define GDT_ENTRY_INIT(flags, base, limit) \
	{                                      \
		.limit0 = (uint16_t)limit,         \
		.limit1 = (limit >> 16) & 0xF,     \
		.base0  = (uint16_t)base,          \
		.base1  = (base >> 16) & 0xFF,     \
		.base2  = (base >> 24) & 0xFF,     \
                                           \
		.type   = flags & 0xF,             \
		.s      = (flags >> 4) & 0x1,      \
		.dpl    = (flags >> 5) & 0x3,      \
		.p      = (flags >> 7) & 0x1,      \
                                           \
		.avl    = (flags >> 12) & 0x1,     \
		.l      = (flags >> 13) & 0x1,     \
		.db     = (flags >> 14) & 0x1,     \
		.g      = (flags >> 15) & 0x1      \
	}

struct gdt_page {
	struct segment_descriptor gdt[GDT_ENTRIES];
} __align(PAGE_SIZE);

struct tss_data {
	uint32_t _reserved_0x00;
	uint64_t rsp[3];
	uint64_t _reserved_0x28;
	uint64_t ist[7];
	uint64_t _reserved_0x92;
	uint16_t _reserved_0x100;
	uint16_t iomba;
} __packed;
#define TSS_ENTRY_INIT() \
	GDT_ENTRY_INIT((uint64_t)(SEGMENT_PRESENT | SEGMENT_TSS), \
	0x00, sizeof(struct tss_data))

struct desc_ptr {
	uint16_t size;
	uint64_t address;
} __packed;

#endif