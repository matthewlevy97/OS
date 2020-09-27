#pragma once

#include <init/multiboot.h>
#include <mm/paging.h>
#include <atomic.h>
#include <common.h>
#include <list.h>
#include <types.h>
#include <stdbool.h>
#include <stdint.h>

#define PMM_INVALID_PAGE               ((uintptr_t)-1)
#define PMM_PAGE_FREE_LIST_SIZE        (32)

struct page {
    uintptr_t physical_page;
    atomic_t count;
};

enum page_range_flags {
    DUMMY_VALUE
};

struct page_range {
    // TODO: Add spinlock on this structure for getting pages
    struct page_range *next;
    struct page *first_page;
    phys_addr_t base, bound;
    enum page_range_flags flags;
    atomic_t count;
};

#define PMM_FREE_LIST_EMPTY(list) \
    ((list).len == 0)
#define PMM_FREE_LIST_FULL(list)  \
    ((list).len == PMM_PAGE_FREE_LIST_SIZE)
struct page_free_list {
    uint32_t len;
    struct page *free_list[PMM_PAGE_FREE_LIST_SIZE];
};

struct page_zone {
    struct page_range *head;
    struct page_free_list list;
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

struct pmm_ram {
    struct page_zone zones[PMM_ZONE_NUM];
    size_t total, total_available;
};

bool pmm_init(multiboot_header_t);
size_t pmm_total_ram();
size_t pmm_total_available_ram();

uintptr_t pmm_get_page();
uintptr_t pmm_get_page_in_zone(pmm_zone_t);

void pmm_acquire_page(phys_addr_t);
void pmm_release_page(phys_addr_t);

struct page *pmm_get_page_for_address(phys_addr_t);

void pmm_insert_range(pmm_zone_t, phys_addr_t, phys_addr_t,
    enum page_range_flags);