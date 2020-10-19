#pragma once

#include <stddef.h>

typedef enum {
    GFP_KERNEL              = 1 << 0,   // Allocate normal kernel data
    GFP_USER                = 1 << 1,   // Allocate ring3 data
    GFP_ETERNAL             = 1 << 2,
} gfp_t;

#define kmalloc(size, flags) kmalloc_a((size), 0, (flags))

void *kmalloc_a(size_t size, size_t align, gfp_t flags);

void free(void *ptr);