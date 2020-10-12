#pragma once

#include <init/multiboot.h>
#include <mm/buddy.h>
#include <mm/paging.h>
#include <atomic.h>
#include <bitmap.h>
#include <common.h>
#include <list.h>
#include <types.h>
#include <stdbool.h>
#include <stdint.h>

#define PMM_INVALID_PAGE               ((uintptr_t)-1)

struct page {
    uintptr_t physical_page;
    atomic_t count;
};

#define PMM_MEMORY_BOUND_DMA           ((uintptr_t)(1 * MB))
#define PMM_MEMORY_BOUND_NORMAL        ((uintptr_t)(4 * GB))
typedef enum {
    PMM_ZONE_DMA,
    PMM_ZONE_NORMAL,
    PMM_ZONE_HIGH,
    PMM_ZONE_NUM,
} pmm_zone_t;

typedef enum {
    __GFP_DMA        = (1 << 0), // PMM_ZONE_LOW
    __GFP_HIGH       = (1 << 1),
    __GPF_ZERO       = (1 << 2), // Zero pages before returning
} pmm_gpf_t;

#define PMM_FREE_LIST_MAX_ORDER        (19)
/**
 * Free lists:
 *  list[0]  = 2GiB blocks [(1 << 19) * PAGE_SIZE]
 *  list[1]  = 1GiB blocks [(1 << 18) * PAGE_SIZE]
 *  list[18] = 8KiB blocks [(1 << 1)  * PAGE_SIZE]
 *  list[19] = 4KiB blocks [(1 << 0)  * PAGE_SIZE]
 */
struct pmm_zone {
    list_entry_t free_list[PMM_FREE_LIST_MAX_ORDER + 1];
    bitmap_t     split_blocks;
    uintptr_t    base_addr;
    size_t       max_order; // Max order being used. Speed up scanning by a bit
};

struct pmm_ram {
    struct pmm_zone zones[PMM_ZONE_NUM];
    size_t total, total_available, used;
};

bool pmm_init(multiboot_header_t);
size_t pmm_total_ram();
size_t pmm_total_available_ram();
size_t pmm_used();

struct page *alloc_page(pmm_gpf_t mask);
vm_addr_t get_free_page(pmm_gpf_t mask);

void free_page(vm_addr_t);

struct page *pmm_get_page_for_address(phys_addr_t);