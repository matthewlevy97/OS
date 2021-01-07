#include <amd64/asm/boot.h>
#include <amd64/asm/common.h>
#include <amd64/asm/msr.h>
#include <amd64/paging.h>
#include <mm/vm_layout.h>

.intel_syntax noprefix

.extern boot_p4
.extern boot_gdt_ptr
.extern kmain

.section .text

.code32
.global _start
_start:
    cli
    cld

    /* Save multiboot information */
    mov edi, eax
	mov esi, ebx

    /* Enable PAE and PGE */
    mov eax, cr4
    or eax, (CR4_PAE | CR4_PGE)
    mov cr4, eax

    /* Load Page Table */
    mov eax, offset (boot_p4 - KERNEL_OFFSET)
    mov cr3, eax

    /* Enable Long Mode */
    mov ecx, MSR_EFER
    rdmsr
    or eax, (EFER_LME | EFER_NXE)
    wrmsr

    /* Enable Paging */
    mov eax, cr0
    or eax, CR0_ENABLE_PAGING
    mov cr0, eax
    /* AMD64 instruction manual states next instruction MUST be a branch */
    jmp 1f
1:
    lgdt [(boot_gdt_ptr - KERNEL_OFFSET)]
    jmp 0x08:(.long_mode_trampoline - KERNEL_OFFSET)

.code64
.long_mode_trampoline:
    xor eax, eax
    mov ss, eax
    mov ds, eax
    mov es, eax
    mov fs, eax
    mov gs, eax

    movabs rax, offset .long_mode
    jmp rax
.long_mode:
    /* Setup stack frame */
    movabs rsp, offset _stack_top
    xor ebp, ebp

    /* Update multiboot pointer to higher-half mapping */
    mov rax, KERNEL_OFFSET
    add rsi, rax
    
    /* Remove identity mapping */
    xor rax, rax
    movabs [boot_p4], rax
    
    /* Reload page table */
    mov rax, cr3
    mov cr3, rax

    /* Reload GDT */
    movabs rax, offset boot_gdt_ptr
    lgdt [rax]

	/* Clear out selectors */
    xor rax, rax
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    /* Reload CS */
    movabs rax, offset 2f
    pushq 0x8
    push rax
    retfq
2:
    /* Jump to C-land */
    movabs rax, offset kmain
    call rax

    cli
    hlt
    jmp $

.section .bss
.align PAGE_SIZE
_stack_bottom:
.skip EARLY_BOOT_STACK_SIZE
_stack_top: