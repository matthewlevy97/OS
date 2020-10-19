
.intel_syntax noprefix

.section .text

.global paging_set_cr3
.global paging_get_cr3
.global paging_invlpg

paging_set_cr3:
    mov cr3, rdi
    ret

paging_get_cr3:
    mov rax, cr3
    ret

paging_invlpg:
    invlpg [rdi]
    ret