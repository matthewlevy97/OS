#include <amd64/paging.h>
#include <mm/kmalloc.h>
#include <mm/paging.h>
#include <mm/pmm.h>
#include <elf.h>
#include <klog.h>
#include <stdint.h>
#include <string.h>

static struct vm_map kernel_vmmap;

static void set_flags(page_table_entry_t*, pg_map_flags_t);
static void setup_kernel_protections(multiboot_header_t);
static void kernel_protection_set_range(uintptr_t, uintptr_t, pg_map_flags_t);

struct vm_map *paging_init(multiboot_header_t multiboot_data)
{
    kernel_vmmap.vm_ranges = NULL;
    kernel_vmmap.page_dir  = paging_get_cr3();
    
    setup_kernel_protections(multiboot_data);

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
        KPANIC("map->page_dir == NULL");
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

static void setup_kernel_protections(multiboot_header_t multiboot_data)
{
    struct multiboot_elf_symbols_tag *elf_symbols;
    pg_map_flags_t flags;
    Elf64_Shdr *section_header;

    elf_symbols = (struct multiboot_elf_symbols_tag*)multiboot_get_tag(
        multiboot_data, MULTIBOOT_TYPE_ELF_SYMBOLS);
    if(NULL == elf_symbols)
        return;
    
    section_header = (Elf64_Shdr*)elf_symbols->elf_section_headers;
    for(size_t i = 0; i < elf_symbols->num_entries; i++, section_header++) {
        if(!(section_header->sh_flags & SHF_ALLOC)) continue;

        flags = PAGE_MAP_PRESENT | PAGE_MAP_GLOBAL_PAGES;
        if(section_header->sh_flags & SHF_WRITE)
            flags |= PAGE_MAP_RW;
        if(!(section_header->sh_flags & SHF_EXECINSTR))
            flags |= PAGE_MAP_NX;
        
        kernel_protection_set_range(
            section_header->sh_addr,
            section_header->sh_addr + section_header->sh_size,
            flags);
    }
}

static void kernel_protection_set_range(uintptr_t base, uintptr_t bound,
    pg_map_flags_t flags)
{
    page_table_t p4, p3, p2, p1;
    page_table_entry_t entry;

    entry.addr = (vm_addr_t)base;
    p4 = (page_table_t)P2V(kernel_vmmap.page_dir);
    if(NULL == p4) {
        KPANIC("kernel_vmmap.page_dir == NULL");
    }
    
    if(NULL == (void*)p4[entry.pml4e].addr) {
        KPANIC("Base P3 not found looking for kernel pages");
    }
    p3 = (page_table_t)P2V(PAGING_GET_PHYSICAL_ADDRESS(p4[entry.pml4e]));

    if(NULL == (void*)p3[entry.pdpe].addr) {
        KPANIC("Base P2 not found looking for kernel pages");
    }
    p2 = (page_table_t)P2V(PAGING_GET_PHYSICAL_ADDRESS(p3[entry.pdpe]));

    for(vm_addr_t page = base; page <= bound; page += PAGE_SIZE) {
        entry.addr = page;
        
        // Should be safe to assume not needing to switch p4 entry
        if(0 == entry.pdpe) {
            if(NULL == (void*)p4[entry.pml4e].addr) {
                KPANIC("P3 not found looking for kernel pages");
            }
            p3 = (page_table_t)P2V(PAGING_GET_PHYSICAL_ADDRESS(p4[entry.pml4e]));
        }
        if(0 == entry.pde) {
            if(NULL == (void*)p3[entry.pdpe].addr) {
                KPANIC("P2 not found looking for kernel pages");
            }
            p2 = (page_table_t)P2V(PAGING_GET_PHYSICAL_ADDRESS(p3[entry.pdpe]));
        }

        if(NULL == (void*)p2[entry.pde].addr) {
            KPANIC("P1 not found looking for kernel pages");
        }
        p1 = (page_table_t)P2V(PAGING_GET_PHYSICAL_ADDRESS(p2[entry.pde]));
        set_flags(&(p1[entry.pte]), flags);
    }
}