#include <mm/kmalloc.h>
#include <mm/pmm.h>
#include <mm/paging.h>
#include <amd64/paging.h>
#include <klog.h>
#include <string.h>

static struct page_range init_ranges[32]; // TODO: Should this become a stack???
static size_t init_ranges_index;

static struct pmm_ram ram;

static void create_regions(struct multiboot_mmap_tag *mboot_mmap);
static bool create_pages(uintptr_t *pg_tbl_base, uintptr_t *pg_tbl_bound);
static void init_page_zone(pmm_zone_t, uintptr_t*);

static bool update_loaded_regions(multiboot_header_t, uintptr_t, uintptr_t);
static bool update_loaded_pages(uintptr_t base, uintptr_t bound);

static struct page_range *get_page_range(uintptr_t base, uintptr_t bound);
static void internal_pmm_insert_range(pmm_zone_t, struct page_range *);

static inline struct page *find_free_page(struct page_range**, struct page**);

static struct page_range *get_init_range(size_t, size_t);

bool pmm_init(multiboot_header_t multiboot_data)
{
    struct multiboot_mmap_tag *mboot_mmap;
    uintptr_t page_table_base, page_table_bound;

    mboot_mmap = (struct multiboot_mmap_tag *)multiboot_get_tag(
        multiboot_data, MULTIBOOT_TYPE_MEMORY_MAP
    );
    if(NULL == mboot_mmap) {
        klog("No memory map found in multiboot data!");
        return false;
    }

    init_ranges_index = 0;

    for(int i = 0; i < PMM_ZONE_NUM; i++) {
        ram.zones[i].head = NULL;
        ram.zones[i].map  = NULL;
    }
    ram.total           = 0;
    ram.total_available = 0;

    if(MULTIBOOT_TYPE_MEMORY_MAP != mboot_mmap->header.type) {
        klog("Expected 'struct multiboot_mmap_tag', found tag of type: %d\n",
            mboot_mmap->header.type);
        return false;
    }

    create_regions(mboot_mmap);

    if(!create_pages(&page_table_base, &page_table_bound))
        return false;
    
    if(!update_loaded_regions(multiboot_data, page_table_base, page_table_bound))
        return false;

    // Ensure free lists are correct
    for(pmm_zone_t zone = PMM_ZONE_LOW; zone <= PMM_ZONE_HIGH; zone++) {
        populate_free_page_list(&(ram.zones[zone]));
    }

    return true;
}

size_t pmm_total_ram()
{
    return ram.total;
}

size_t pmm_total_available_ram()
{
    return ram.total_available;
}

struct page *alloc_page(pmm_gpf_t mask)
{
    // TODO: 
}

struct page *alloc_pages(pmm_gpf_t mask, uint32_t order)
{
    // TODO:
}

uintptr_t get_free_page(pmm_gpf_t mask)
{
    // TODO:
}

void pmm_acquire_page(phys_addr_t physical_address)
{
    struct page *p;

    p = pmm_get_page_for_address(physical_address);
    if(p) {
        atomic_inc(&(p->count));
    }
}

void pmm_release_page(phys_addr_t physical_address)
{
    struct page *p;
    pmm_zone_t zone;

    p = pmm_get_page_for_address(physical_address);
    if(p) {
        if(atomic_dec_and_test(&(p->count))) {
            if(p->physical_page < PMM_LOW_MEMORY)
                zone = PMM_ZONE_LOW;
            else if(p->physical_page < PMM_NORMAL_MEMORY)
                zone = PMM_ZONE_NORMAL;
            else
                zone = PMM_ZONE_HIGH;
            
            // TODO: Make page avaliable to be allocated again
        }
    }
}

struct page *pmm_get_page_for_address(phys_addr_t physical_address)
{
    struct page_range *range;
    struct page *page;
    size_t index;
    
    physical_address = physical_address & ~(PAGE_SIZE - 1);
    range = get_page_range(physical_address, physical_address);
    if(!range) return NULL;

    page  = range->first_page;
    index = (physical_address - range->base) / PAGE_SIZE;

    return page + index;
}

static void create_regions(struct multiboot_mmap_tag *mboot_mmap)
{
    struct multiboot_mmap_entry_tag *entry;
    size_t num_entries;
    uintptr_t base, bound;

    // Create regions for all reserved and available RAM regions
    num_entries = (mboot_mmap->header.size - sizeof(*mboot_mmap)) /
        (mboot_mmap->entry_size);
    for(size_t i = 0; i < num_entries; i++) {
        entry = &mboot_mmap->entries[i];
        ram.total += entry->length;

        switch(entry->type)
        {
        case MULTIBOOT_MMAP_TYPE_AVAILABLE:
            base  = entry->base_addr;
            bound = base + entry->length;

            if(base < PMM_LOW_MEMORY) {
                if(bound > PMM_LOW_MEMORY) {
                    internal_pmm_insert_range(PMM_ZONE_LOW, get_init_range(
                        base, PMM_LOW_MEMORY - 1
                    ));
                    // Setup to create a region in zone_normal
                    base = PMM_LOW_MEMORY;
                } else {
                    internal_pmm_insert_range(PMM_ZONE_LOW, get_init_range(
                        base, bound
                    ));
                }
            }

            if(base >= PMM_LOW_MEMORY && base < PMM_NORMAL_MEMORY) {
                if(bound > PMM_NORMAL_MEMORY) {
                    internal_pmm_insert_range(PMM_ZONE_NORMAL, get_init_range(
                        base, PMM_NORMAL_MEMORY - 1
                    ));
                    // Setup to create a region in zone_high
                    base = PMM_NORMAL_MEMORY;
                } else {
                    internal_pmm_insert_range(PMM_ZONE_NORMAL, get_init_range(
                        base, bound
                    ));
                }
            }
            
            if(base >= (size_t)PMM_NORMAL_MEMORY) {
                internal_pmm_insert_range(PMM_ZONE_HIGH, get_init_range(
                    base, bound
                ));
            }

            ram.total_available += entry->length;
            break;
        case MULTIBOOT_MMAP_TYPE_RESERVE:
            internal_pmm_insert_range(PMM_ZONE_RESERVED, get_init_range(
                entry->base_addr, entry->base_addr + entry->length
            ));
            break;
        }
    }
}

static bool create_pages(uintptr_t *pg_tbl_base, uintptr_t *pg_tbl_bound)
{
    struct page_range *range;
    size_t num_entries;

    // Create all 'struct pages' for available RAM
    num_entries = (ram.total_available / PAGE_SIZE);
    range = ram.zones[PMM_ZONE_NORMAL].head;
    while(range) {
        if(range->bound - range->base > num_entries)
            break;
        range = range->next;
    }
    /*
    * We should cleanup, but returning false here means PMM didn't initialize
    * so the kernel just shits the bed and halts
    */
    if(!range)
        return false;
    
    *pg_tbl_base  = (uintptr_t)P2V(range->base);
    *pg_tbl_bound = *pg_tbl_base;

    // Link ranges back to underlying pages
    init_page_zone(PMM_ZONE_LOW, pg_tbl_bound);
    init_page_zone(PMM_ZONE_NORMAL, pg_tbl_bound);
    init_page_zone(PMM_ZONE_HIGH, pg_tbl_bound);

    return true;
}

static void init_page_zone(pmm_zone_t zone, uintptr_t *page_table_base)
{
    struct page_range *range;
    struct page *page;
    uintptr_t base, bound;

    range = ram.zones[zone].head;
    while(range) {
        range->first_page = (struct page*)(*page_table_base);

        // Initialize pages in range
        page  = range->first_page;
        base  = range->base / PAGE_SIZE;
        bound = range->bound / PAGE_SIZE;
        for(size_t i = base; i < bound; i++) {
            atomic_set(&(page->count), 0);
            page->physical_page = i * PAGE_SIZE;

            // TODO: Add page to list of free pages

            page++;
        }

        range = range->next;
        *page_table_base = (uintptr_t)page;
    }
}

static bool update_loaded_regions(multiboot_header_t multiboot_data,
    uintptr_t page_table_base, uintptr_t page_table_bound)
{
    // Increase reference count for multiboot pages
    if(!update_loaded_pages(V2P(multiboot_data),
        V2P(multiboot_data) + multiboot_data->size))
        return false;

    // TODO: Increase reference count for loaded modules
    //multiboot_get_tag(multiboot_data, MULTIBOOT_TYPE_MEMORY_MAP);

    // Increase reference count for kernel pages
    if(!update_loaded_pages(V2P(&_kernel_start), V2P(&_kernel_end)))
        return false;
    
    // Increase reference count for 'struct page' table
    if(!update_loaded_pages(V2P(page_table_base), V2P(page_table_bound)))
        return false;

    return true;
}

static bool update_loaded_pages(uintptr_t base, uintptr_t bound)
{
    struct page_range *range;
    struct page *page;
    size_t num_pgs;
    size_t index;

    num_pgs = ((bound - base) / PAGE_SIZE) +
        (((bound - base) % PAGE_SIZE) ? 1 : 0);

    range = get_page_range(base, bound);
    if(range) {
        if(NULL == range->first_page) {
            klog("range->first_page == NULL");
            return false;
        }

        index = (base - range->base) / PAGE_SIZE;
        page = range->first_page + index;
        
        while(num_pgs--) {
            atomic_inc(&(page->count));
            page++;
        }

        page = range->first_page + index;

        return true;
    }
    return false;
}

static struct page_range *get_page_range(uintptr_t base, uintptr_t bound)
{
    struct page_range *range;
    pmm_zone_t zone;

    if(base < PMM_LOW_MEMORY)
        zone = PMM_ZONE_LOW;
    else if(base < PMM_NORMAL_MEMORY)
        zone = PMM_ZONE_NORMAL;
    else
        zone = PMM_ZONE_HIGH;

    range = ram.zones[zone].head;
    while(range) {
        if(range->base <= base && range->bound >= bound)
            break;
        range = range->next;
    }

    return range;
}

/**
 * TODO: Handle splitting of regions
 */
static void internal_pmm_insert_range(pmm_zone_t zone, struct page_range *range)
{
    struct page_range **current;

    range->next  = NULL;
    atomic_set(&(range->count), 0);

    current = &(ram.zones[zone].head);
    while(*current) {
        // Sort by base
        if(range->base < (*current)->base) {
            range->next = *current;
            break;
        }
        current = &((*current)->next);
    }
    *current = range;
    atomic_inc(&(range->count));
}

static inline struct page *find_free_page(struct page_range **range,
    struct page **page)
{
    return NULL;
}

static inline struct page_range *get_init_range(size_t base, size_t bound)
{
    init_ranges[init_ranges_index].base            = base;
    init_ranges[init_ranges_index].bound           = bound;
    init_ranges[init_ranges_index].first_page      = NULL;
    init_ranges_index++;

    return &(init_ranges[init_ranges_index-1]);
}