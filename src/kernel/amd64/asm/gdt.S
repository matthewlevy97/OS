#include <amd64/asm/segments.h>

.intel_syntax noprefix

.section .text

.global gdt_reload
.global tss_install

gdt_reload:
    lgdt [rdi]

    /* Clear out selectors */
    xor rax, rax
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    /* Reload CS */
    popq rax
    pushq __KERNEL_CS
    push rax
    retfq

tss_install:
    mov ax, __TSS_SELECTOR
    ltr ax
    ret