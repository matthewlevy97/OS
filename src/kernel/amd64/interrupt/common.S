#include <amd64/cpu.h>

.intel_syntax noprefix

.extern interrupt_handler

.global isr_common
.global isr_common_return_path
.global idt_load
.global check_interrupts_enabled
.global interrupt_disable_flag
.global interrupt_enable_flag

.section .text
isr_common:
    /* Save all registers */
    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8
    push rbp
    push rdi
    push rsi
    push rdx
    push rcx
    push rbx
    push rax

    mov rax, cr4
    push rax
    mov rax, cr3
    push rax
    mov rax, cr2
    push rax
    mov rax, cr0
    push rax

    cld
    mov rdi, rsp
    call interrupt_handler
    /* Hacky way to context switch. Just need to ensure rax=stack_pointer */
isr_common_return_path:
    mov rsp, rax

    /* Clear CRX values */
    add rsp, 0x20

    pop rax
    pop rbx
    pop rcx
    pop rdx
    pop rsi
    pop rdi
    pop rbp
    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15

    add rsp, 0x10
    iretq

idt_load:
    lidt [rdi]
    ret

check_interrupts_enabled:
    pushfq
    pop rax
    and rax, X86_EFLAGS_IF
    ret

interrupt_disable_flag:
    pushfq
    cli
    pop rax
    ret

interrupt_enable_flag:
    push rdi
    popf
    ret

interrupt_enable:
    sti
    ret

interrupt_disable:
    cli
    ret