#pragma once

#include <list.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * Paging Functionality To Add
 * ---------------------------
 * page_fault_handler()
 * paging_invalidate()
 * paging_reload_dir()
 * paging_map()
 * paging_map_range()
 */

/*
 * Note: The actual implementation for this code can be found under the
 *  architecture folder the OS was compiled for (ie. amd64). This is done to
 *  make porting to other systems easier as the API can be described here
 *  and implemented specificly for the system being targeted elsewhere.
*/

typedef uintptr_t page_dir_t;
typedef uintptr_t vm_addr_t;
typedef uintptr_t phys_addr_t;

typedef enum {
    PAGE_MAP_SEQUENTIAL_PHYSICAL_PAGES = 1 << 0, // For DMA, etc.
    PAGE_MAP_GLOBAL_PAGES              = 1 << 1, // Updates to CR3 won't change
    PAGE_MAP_RW                        = 1 << 2,
    PAGE_MAP_NX                        = 1 << 3,
    PAGE_MAP_RING3                     = 1 << 4,
} pg_map_flags_t;

struct virtual_range {
    ListEntry_HEAD
    vm_addr_t vm_start, vm_end;
    struct page_range *physical_range;
    pg_map_flags_t vm_flags;
};

struct vm_map {
    page_dir_t page_dir;
    struct virtual_range *vm_ranges;
};

bool paging_init();

bool paging_map_page(struct vm_map*, vm_addr_t);
bool paging_map_page2(struct vm_map*, vm_addr_t, phys_addr_t, pg_map_flags_t);

// Note: Needs to handle splitting of ranges and creating new ones
bool paging_map_range(struct vm_map*, struct virtual_range*);

bool paging_unmap_page(struct vm_map*, vm_addr_t);
bool paging_unmap_range(struct vm_map*, struct virtual_range*);

bool paging_set_flags(struct vm_map*, vm_addr_t, pg_map_flags_t);
bool paging_set_range(struct vm_map*, struct virtual_range*, pg_map_flags_t);

void paging_make_canonical(vm_addr_t*);