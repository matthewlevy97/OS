.intel_syntax noprefix

.section .text

.code32
.global _start
_start:
    cli

    hlt
    jmp $