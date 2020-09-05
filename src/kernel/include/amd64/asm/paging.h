#pragma once

#define KERNEL_OFFSET                 (0xFFFFFF8000000000ULL)
#define PAGE_SIZE                     (0x1000)
#define ENTRIES_PER_PT                (512)

#ifdef __ASSEMBLER__
#define V2P(a)                        ((a) - KERNEL_OFFSET)
#define P2V(a)                        ((a) + KERNEL_OFFSET)
#else
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
#define PAGE_HUGE                     (1 << 7)
#define PAGE_GLOBAL                   (1 << 8)
#define PAGE_NX                       (1 << 63)