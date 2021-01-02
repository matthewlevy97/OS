#include <amd64/paging.h>
#include <mm/kmalloc.h>
#include <mm/paging.h>
#include <mm/pmm.h>

#include <stdint.h>
#include <string.h>

static struct vm_map kernel_vmmap;
static void set_flags(page_table_entry_t*, pg_map_flags_t);

struct vm_map *paging_init()
{
    size_t kernel_size;

    kernel_vmmap.vm_ranges = NULL;
    kernel_vmmap.page_dir  = paging_get_cr3();

    kernel_size = ((uintptr_t)&_kernel_end) - ((uintptr_t)&_kernel_start);
    
    // TODO: Might want to remap kernel to enable better protections (Enable NX, RO, etc.)

    return &kernel_vmmap;
}

void paging_reload_tlb(struct vm_map *map)
{
    paging_set_cr3(map->page_dir);
}

struct vm_map *paging_create_vmmap()
{
    struct vm_map *vm_map;
    page_table_t src, dst;

    vm_map = kmalloc(sizeof(*vm_map), GFP_KERNEL | GFP_ZERO);
    if(NULL == vm_map)
        return NULL;

    vm_map->page_dir = get_free_page(__GPF_ZERO);

    // TODO: Copy over vm_ranges
    vm_map->vm_ranges = NULL;
    
    // Copy upper half of page directory
    src = P2V(kernel_vmmap.page_dir);
    dst = P2V(vm_map->page_dir);
    for(size_t i = PAGING_ENTRIES_PER_PT / 2; i < PAGING_ENTRIES_PER_PT; i++) {
        dst[i] = src[i];
    }

    return vm_map;
}
void paging_destroy_vmmap(struct vm_map **vm_map)
{
    if(NULL == vm_map || NULL == *vm_map)
        return;
    
    // TODO: Free vm_ranges and unmap regions as doing so
}

void paging_map(struct vm_map *map, vm_addr_t addr, pg_map_flags_t flags)
{
    paging_map2(map, addr, get_free_page(__GPF_ZERO), flags);
}

void paging_map2(struct vm_map *map, vm_addr_t addr, phys_addr_t paddr,
    pg_map_flags_t flags)
{
    page_table_entry_t entry;
    page_table_t table;

    if(NULL == map) {
        return;
    }

    // TODO: Update map

    entry.addr = addr;
    table = (page_table_t)P2V(map->page_dir);

    if(NULL == table) {
        // TODO: PANIC
        return;
    }
    
    if(NULL == (void*)table[entry.pml4e].addr) {
        table[entry.pml4e].addr = get_free_page(__GPF_ZERO);
        set_flags(&table[entry.pml4e], flags);
    }
    table = (page_table_t)P2V(PAGING_GET_PHYSICAL_ADDRESS(table[entry.pml4e]));

    if(NULL == (void*)table[entry.pdpe].addr) {
        table[entry.pdpe].addr = get_free_page(__GPF_ZERO);
        set_flags(&table[entry.pdpe], flags);
    }
    table = (page_table_t)P2V(PAGING_GET_PHYSICAL_ADDRESS(table[entry.pdpe]));

    if(NULL == (void*)table[entry.pde].addr) {
        table[entry.pde].addr = get_free_page(__GPF_ZERO);
        set_flags(&table[entry.pde], flags);
    }
    table = (page_table_t)P2V(PAGING_GET_PHYSICAL_ADDRESS(table[entry.pde]));

    if(NULL == (void*)table[entry.pte].addr || (flags & PAGE_MAP_OVERWRITE)) {
        table[entry.pte].addr = ALIGN_PAGE(paddr);
        set_flags(&table[entry.pte], flags);
    }
}

void paging_map_range(struct vm_map *map, vm_addr_t base, size_t length,
    pg_map_flags_t flags)
{
    paging_map_range2(map, base, get_pages(length, __GPF_ZERO), length, flags);
}

void paging_map_range2(struct vm_map *map, vm_addr_t base, phys_addr_t phys_base,
    size_t length, pg_map_flags_t flags)
{
    length = ALIGN_PAGE(length);
    while(length > 0) {
        paging_map2(map, base, phys_base, flags);

        base      += PAGE_SIZE;
        phys_base += PAGE_SIZE;
        length    -= PAGE_SIZE;
    }
}

void paging_unmap(struct vm_map *map, vm_addr_t addr)
{
    // TODO: Update map

    paging_invlpg(addr);
}

void paging_unmap_range(struct vm_map *map, vm_addr_t base, size_t length,
    pg_map_flags_t flags)
{
    length = ALIGN_PAGE(length);
    while(length > 0) {
        paging_unmap(map, base);

        base   += PAGE_SIZE;
        length -= PAGE_SIZE;
    }
}

void paging_make_canonical(vm_addr_t *addr)
{
    if((*addr >> 47) & 0x1) {
        *addr |= (0xFFFFULL << 48);
    } else {
        *addr &= ~(0xFFFFULL << 48);
    }
}

static __inline void set_flags(page_table_entry_t *entry, pg_map_flags_t flags)
{
    if(flags & PAGE_MAP_PRESENT)
        entry->present = 1;
    if(flags & PAGE_MAP_RW)
        entry->writable = 1;
    if(flags & PAGE_MAP_NX)
        entry->nx = 1;
    if(flags & PAGE_MAP_RING3)
        entry->usermode = 1;
    if(flags & PAGE_MAP_GLOBAL_PAGES)
        entry->global = 1;
}