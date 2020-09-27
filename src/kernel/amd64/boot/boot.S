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
    mov eax, offset V2P(boot_p4)
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
    lgdt [V2P(boot_gdt_ptr)]
    jmp 0x08:V2P(.long_mode_trampoline)

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

    /* Map 1GB physical memory to virtual */
    call boot_update_paging_pae_enabled
    
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

/*
This maps 1GB of physical memory -> virtual memory starting at kernel base
    Convert boot_p2 from pointing to 4KB PTEs -> 2MB PDE
TODO: Convert boot_p1 -> 2MB PDE also
*/
boot_update_paging_pae_enabled:
    /* Get P3 */
    movabs rax, [boot_p4 + 8 * EARLY_BOOT_HIGHER_HALF_INDEX]
    and rax, (~0xFFF)
    
    /* Get P2 */
    mov rax, [rax]
    and rax, (~0xFFF)

    /* Fill with 2MB page table entries */
    xor rcx, rcx
1:
    cmp rcx, (ENTRIES_PER_PT - 1)
    jge 2f

    mov rdx, rcx
    shl rdx, 20
    or  rdx, (PAGE_HUGE | PAGE_PRESENT | PAGE_WRITE)

    mov [rax], rdx

    add rax, 8
    inc rcx
    jmp 1b
2:
    ret

.section .bss
.align PAGE_SIZE
_stack_bottom:
.skip EARLY_BOOT_STACK_SIZE
_stack_top: