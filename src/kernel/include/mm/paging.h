#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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
    PAGE_MAP_SEQUENTIAL                = 1 << 0,
    PAGE_MAP_ETERNAL                   = 1 << 1,
} pg_map_flags_t;

bool paging_init();

void page_dir_set_active(page_dir_t*);

page_dir_t *page_dir_clone(page_dir_t*);
page_dir_t *page_dir_create(void);
int         page_dir_destroy(page_dir_t*);

page_dir_t *paging_map_page(page_dir_t*, vm_addr_t);
page_dir_t *paging_map_page2(page_dir_t*, vm_addr_t, phys_addr_t);
page_dir_t *paging_map_range(page_dir_t*, vm_addr_t, size_t num_pages);
page_dir_t *paging_map_range2(page_dir_t*, vm_addr_t, size_t, pg_map_flags_t);

page_dir_t *paging_unmap_page(page_dir_t*, vm_addr_t);
page_dir_t *paging_unmap_range(page_dir_t*, vm_addr_t start, vm_addr_t end);

void paging_make_canonical(vm_addr_t*);