#include <amd64/paging.h>
#include <mm/paging.h>
#include <mm/kmalloc.h>

#include <stdint.h>

static uint32_t get_p4_index(uintptr_t);
static uint32_t get_p3_index(uintptr_t);
static uint32_t get_p2_index(uintptr_t);
static uint32_t get_p1_index(uintptr_t);

bool paging_init()
{
    return true;
}

void page_dir_set_active(page_dir_t *pg_dir)
{

}

page_dir_t *page_dir_clone(page_dir_t *pg_dir)
{
    return NULL;
}

page_dir_t *page_dir_create()
{
    return NULL;
}

int page_dir_destroy(page_dir_t *pg_dir)
{
    return 0;
}

page_dir_t *paging_map_page(page_dir_t *pg_dir, vm_addr_t vaddr)
{
    return NULL;
}

page_dir_t *paging_map_page2(page_dir_t *pg_dir,
    vm_addr_t vaddr, phys_addr_t paddr)
{
    return NULL;
}

page_dir_t *paging_map_range(page_dir_t *pg_dir,
    vm_addr_t vaddr, size_t num_pages)
{
    return NULL;
}

page_dir_t *paging_map_range2(page_dir_t *pg_dir, vm_addr_t vaddr,
    size_t num_pages, pg_map_flags_t flags)
{
    return NULL;
}

page_dir_t *paging_unmap_page(page_dir_t *pg_dir, vm_addr_t vaddr)
{
    return NULL;
}

page_dir_t *paging_unmap_range(page_dir_t *pg_dir,
    vm_addr_t start, vm_addr_t end)
{
    return NULL;
}

void paging_make_canonical(vm_addr_t *addr)
{
    if((*addr >> 47) & 0x1) {
        *addr |= (0xFFFFULL << 48);
    } else {
        *addr &= ~(0xFFFFULL << 48);
    }
}

static inline uint32_t get_p4_index(uintptr_t address)
{
    return (address >> 39) & 511;
}
static __inline uint32_t get_p3_index(uintptr_t address)
{
    return (address >> 30) & 511;
}
static __inline uint32_t get_p2_index(uintptr_t address)
{
    return (address >> 21) & 511;
}
static __inline uint32_t get_p1_index(uintptr_t address)
{
    return (address >> 12) & 511;
}