#pragma once

#define X86_EFLAGS_CF    (1UL << 0)
#define X86_EFLAGS_PF    (1UL << 2)
#define X86_EFLAGS_AF    (1UL << 4)
#define X86_EFLAGS_ZF    (1UL << 6)
#define X86_EFLAGS_SF    (1UL << 7)
#define X86_EFLAGS_TF    (1UL << 8)
#define X86_EFLAGS_IF    (1UL << 9)
#define X86_EFLAGS_DF    (1UL << 10)
#define X86_EFLAGS_OF    (1UL << 11)
/**
 * Allow RING-3 to access I/O Ports
 * This includes *cli* and *sti* because reasons
 */
#define X86_EFLAGS_IOPL  (3UL << 12)
#define X86_EFLAGS_NT    (1UL << 14)
#define X86_EFLAGS_RF    (1UL << 16)
#define X86_EFLAGS_VM    (1UL << 17)
#define X86_EFLAGS_AC    (1UL << 18)
#define X86_EFLAGS_VIF   (1UL << 19)
#define X86_EFLAGS_VIP   (1UL << 20)
#define X86_EFLAGS_ID    (1UL << 21)

#define X86_EFLAGS_STANDARD (\
	X86_EFLAGS_DF | X86_EFLAGS_IF | \
	X86_EFLAGS_AC)

#ifndef __ASSEMBLER__

#include <amd64/asm/segments.h>
#include <stdint.h>

struct cpu_local_data {
    struct desc_ptr gdt_ptr;
	struct gdt_page gdt_page;
	
	struct tss_data tss;
};

typedef struct {
	uint64_t cr0;
	uint64_t cr2;
	uint64_t cr3;
	uint64_t cr4;

	uint64_t rax;
	uint64_t rbx;
	uint64_t rcx;
	uint64_t rdx;
	uint64_t rsi;
	uint64_t rdi;
	uint64_t rbp;
	uint64_t r8;
	uint64_t r9;
	uint64_t r10;
	uint64_t r11;
	uint64_t r12;
	uint64_t r13;
	uint64_t r14;
	uint64_t r15;

	uint64_t int_no;
	uint64_t err_code;

	uint64_t rip;
	uint64_t cs;
	uint64_t rflags;
	uint64_t rsp;
	uint64_t ss;
} registers_t;

#include <amd64/idt.h>

void isr_common(void);
void idt_load(void *);

#endif