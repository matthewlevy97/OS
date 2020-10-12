#pragma once

#include <mm/paging.h>

#define GET_BUDDY(addr, size)                  \
    ((uintptr_t)(addr)) ^ (size))
#define BUDDY_GET_SIZE(order)                  \
    ((1 << (order)) * PAGE_SIZE)
#define BUDDY_TOTAL_TRACKED(max_order)         \
    ((1 << (max_order)) * PAGE_SIZE)
#define BUDDY_INDEX_IN_LEVEL(ptr, base, order) \
    (((uintptr_t)(ptr) - (uintptr_t)(base)) / BUDDY_GET_SIZE(order))
