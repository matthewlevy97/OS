#pragma once

#include <init/multiboot.h>
#include <mm/paging.h>
#include <atomic.h>
#include <common.h>
#include <types.h>
#include <stdbool.h>
#include <stdint.h>

#define PMM_INVALID_PAGE               ((uintptr_t)-1)
#define PMM_PAGE_FREE_LIST_SIZE        (32)

struct page {
    uintptr_t physical_page;
    atomic_t count;
};

struct page_range {
    // TODO: Add spinlock on this structure for getting pages
    struct page_range *next;
    struct page *first_page;
    phys_addr_t base, bound;
    atomic_t count;
};

struct page_zone {
    struct page_range *head;
    struct buddy_bitmap *map;
};

#define PMM_LOW_MEMORY                 (1 * MB)
#define PMM_NORMAL_MEMORY              (4 * GB)
typedef enum {
    PMM_ZONE_RESERVED,
    PMM_ZONE_LOW,
    PMM_ZONE_NORMAL,
    PMM_ZONE_HIGH,
    PMM_ZONE_NUM,
} pmm_zone_t;

typedef enum {
    __GFP_DMA        = (1 << 0), // PMM_ZONE_LOW
    __GFP_HIGH       = (1 << 1),
    __GPF_ZERO       = (1 << 2), // Zero pages before returning
} pmm_gpf_t;

struct pmm_ram {
    struct page_zone zones[PMM_ZONE_NUM];
    size_t total, total_available;
};

bool pmm_init(multiboot_header_t);
size_t pmm_total_ram();
size_t pmm_total_available_ram();

struct page *alloc_page(pmm_gpf_t mask);
struct page *alloc_pages(pmm_gpf_t mask, uint32_t order);
uintptr_t get_free_page(pmm_gpf_t mask);

void pmm_acquire_page(phys_addr_t);
void pmm_release_page(phys_addr_t);

struct page *pmm_get_page_for_address(phys_addr_t);