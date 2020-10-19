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
 * paging_reload_tlb()
 * paging_map()
 * paging_map_range()
 * paging_unmap()
 * paging_unmap_range()
 * paging_make_canonical();
 */

/*
 * Note: The actual implementation for this code can be found under the
 *  architecture folder the OS was compiled for (ie. amd64). This is done to
 *  make porting to other systems easier as the API can be described here
 *  and implemented specificly for the system being targeted elsewhere.
*/

typedef uintptr_t vm_addr_t;
typedef uintptr_t phys_addr_t;

typedef enum {
    PAGE_MAP_PRESENT                   = 1 << 0,
    PAGE_MAP_GLOBAL_PAGES              = 1 << 1, // Updates to CR3 won't change
    PAGE_MAP_RW                        = 1 << 2,
    PAGE_MAP_NX                        = 1 << 3,
    PAGE_MAP_RING3                     = 1 << 4,
    PAGE_MAP_OVERWRITE                 = 1 << 5, // Overwrite mapping if exists
    PAGE_MAP_SEQUENTIAL_PHYSICAL_PAGES = 1 << 6, // For DMA, etc.
} pg_map_flags_t;

enum paging_range_type {
    VM_RANGE_PHYSICAL
};

struct virtual_range {
    // TODO: AVL TREE
    vm_addr_t vm_start, vm_end;
    pg_map_flags_t vm_flags;

    // TODO: Page fault handler

    enum paging_range_type type;
    union {
        void *physical_start;
        // TODO: "struct file *"
    };
};

struct vm_map {
    phys_addr_t page_dir;
    struct virtual_range *vm_ranges;
};

bool paging_init(struct vm_map*);

void paging_reload_tlb(struct vm_map*);

void paging_map(struct vm_map*, vm_addr_t, pg_map_flags_t);
void paging_map2(struct vm_map*, vm_addr_t, phys_addr_t, pg_map_flags_t);

void paging_map_range(struct vm_map*, vm_addr_t, size_t, pg_map_flags_t);
void paging_map_range2(struct vm_map*, vm_addr_t, phys_addr_t, size_t,
    pg_map_flags_t);

void paging_unmap(struct vm_map*, vm_addr_t);
void paging_unmap_range(struct vm_map*, vm_addr_t, size_t, pg_map_flags_t);

void paging_make_canonical(vm_addr_t*);