#pragma once

#include <mm/paging.h>
#include <list.h>
#include <stddef.h>

#define KMALLOC_PAGE_MAP_FLAGS \
    (PAGE_MAP_PRESENT | PAGE_MAP_GLOBAL_PAGES | PAGE_MAP_RW | PAGE_MAP_NX)

/**
 * If the size of a block will be smaller than this, don't split it
 */
#define KMALLOC_MIN_SPLIT_SIZE         (sizeof(size_t))

#define KMALLOC_INIT_FLAGS             (0)

enum kmalloc_flags {
    KMALLOC_IN_USE = 1 << 0,
};

struct kmalloc_preamble {
    ListEntry_HEAD
    size_t blk_size;
    enum kmalloc_flags flags;
    char data[];
} __packed; // TODO: Does this need to be packed??

typedef enum {
    GFP_KERNEL              = 1 << 0,   // Allocate normal kernel data
    GFP_USER                = 1 << 1,   // Allocate ring3 data

    GFP_ZERO                = 1 << 2,   // Zero the memory region

    GFP_ETERNAL             = 1 << 3,   // Use the eternal allocator
} gfp_t;

#define kmalloc(size, flags) kmalloc_a((size), 0, (flags))

bool kmalloc_init(struct vm_map *vm_map);

void *kmalloc_a(size_t size, size_t align, gfp_t flags);

void kfree(void *ptr);