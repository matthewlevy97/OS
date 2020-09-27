#pragma once

#include <mm/vm_layout.h>

#define PAGE_SIZE                     (0x1000)
#define ENTRIES_PER_PT                (512)

#ifdef __ASSEMBLER__

#define V2P(a)                        ((a) - KERNEL_OFFSET)
#define P2V(a)                        ((a) + KERNEL_OFFSET)

#else

#include <stdint.h>

#define V2P(a)                        ((uintptr_t)(a) & ~KERNEL_OFFSET)
#define P2V(a)                        ((void *)((uintptr_t)(a) | KERNEL_OFFSET))

#endif

#define PAGE_PRESENT                  (1)
#define PAGE_WRITE                    (1 << 1)
#define PAGE_USER                     (1 << 2)
#define PAGE_WRITETHROUGH             (1 << 3)
#define PAGE_CACHE_DISABLE            (1 << 4)
#define PAGE_ACCESSED                 (1 << 5)
#define PAGE_DIRTY                    (1 << 6)
/*
PAGE_HUGE - Page 143 of amd64_arch_programming_manual.pdf
- If EFER.LMA=1 and PDPE.PS=1, the physical-page size is 1 Gbyte
- If CR4.PAE=0 and PDE.PS=1, the physical-page size is 4 Mbytes
- If CR4.PAE=1 and PDE.PS=1, the physical-page size is 2 Mbytes.
*/
#define PAGE_HUGE                     (1 << 7)
#define PAGE_GLOBAL                   (1 << 8)
#define PAGE_NX                       (1 << 63)

#ifndef __ASSEMBLER__

typedef union {
    uint64_t entry;
    struct {
        uint8_t  nx                           : 1;
        union {
            struct {
                uint8_t memory_protection_key : 4;
                uint8_t _unused               : 7;
            };
            uint32_t __unused                 : 11;
        };
        uint64_t _physical_address            : 40;
        uint8_t  _available                   : 3;
        uint8_t  global                       : 1;
        uint8_t  page_size                    : 1;
        uint8_t  dirty                        : 1;
        uint8_t  accessed                     : 1;
        uint8_t  disable_cache                : 1;
        uint8_t  writethrough_cache           : 1;
        uint8_t  usermode                     : 1;
        uint8_t  writable                     : 1;
        uint8_t  present                      : 1;
    };
} page_table_entry_t;
#define PAGING_GET_PHYSICAL_ADDRESS(pt_entry) \
    (((pt_entry)._physical_address) << 12)

typedef page_table_entry_t* page_table_t;

#endif