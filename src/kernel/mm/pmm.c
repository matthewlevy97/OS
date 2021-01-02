#include <mm/kmalloc.h>
#include <mm/pmm.h>
#include <mm/paging.h>
#include <common.h>
#include <string.h>
#include <klog.h>

#define V2P_KERNEL_BASED(a) ((uintptr_t)(a) & ~KERNEL_OFFSET)

/**
 * TODO:
 *  Currently capped at total RAM < 1G. To bypass need to dynamically add
 *      page tables to cover entire physical memory space. Need to be done
 *      early (probably before anything else runs)
 *  Keep free lists sorted by base address
 *      Makes checking for buddy a bit faster
 *  Add coalesce-ing code
 *  Code smells bad, might want to go back and improve at a later date
 *  Add code to profile system
 *      - Determine how much RAM is attached, is available, is used by kernel, etc
 *  Think about having the buddys done in reverse order so smaller blocks
 *      appear first closer to the base than the bound. Allows using smaller
 *      addresses before larger ones
 */

static struct pmm_ram ram;

static bool setup_buddy_allocator(multiboot_header_t);
static void insert_ram_region(uintptr_t, uint64_t);
static void insert_region(uintptr_t, uint64_t, pmm_zone_t);
static bool create_split_bitmap();
static void initialize_zone(struct pmm_zone*);
static void buddy_bitmap_init(char *page);

static bool internal_alloc(size_t order, pmm_zone_t, bool use_bitmap);
static bool internal_split_block(list_entry_t, size_t, pmm_zone_t);
static bool buddy_split_block(list_entry_t block, size_t order, pmm_zone_t);

static bool setup_struct_pages();

static size_t get_free_list_index(size_t num_pages);
static size_t get_free_list_index_no_round(size_t num_pages);

bool pmm_init(multiboot_header_t mboot_header)
{
    ram.total           = 0;
    ram.total_available = 0;
    ram.used            = 0;
    for(int i = 0; i < PMM_ZONE_NUM; i++) {
        initialize_zone(&ram.zones[i]);
    }

    if(!setup_buddy_allocator(mboot_header))
        return false;

    if(!setup_struct_pages())
        return false;
    
    /** TODO: Mark kernel, multiboot, and kernel modules as "in-use" pages
     * 
    critical_start = V2P_KERNEL_BASED(&_kernel_start);
    critical_end   = ALIGN_PAGE((uintptr_t)&_kernel_end);
    critical_end  += ALIGN_PAGE(mboot_header->size);
    critical_end   = V2P_KERNEL_BASED(critical_end);
    */
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
size_t pmm_used()
{
    return ram.used;
}

phys_addr_t get_free_page(pmm_gpf_t mask)
{
    return get_pages(PAGE_SIZE, mask);
}

phys_addr_t get_pages(size_t length, pmm_gpf_t mask)
{
    list_entry_t *first_free;
    struct page *struct_page;
    vm_addr_t page;
    pmm_zone_t zone;
    size_t order;

    if(mask & __GFP_DMA)
        zone = PMM_ZONE_DMA;
    else if(mask & __GFP_HIGH)
        zone = PMM_ZONE_HIGH;
    else
        zone = PMM_ZONE_NORMAL;

    order = get_free_list_index(length / PAGE_SIZE);

    first_free = &ram.zones[zone].free_list[order];
    if(NULL == *first_free) {
        if(!internal_alloc(order, zone, true))
            return PMM_INVALID_PAGE;
    }

    // If we are here we found page(s) that can store the bitmap
    page = (uintptr_t)*first_free;
    *first_free = (*first_free)->next;

    ram.used += length;

    if(mask & __GPF_ZERO)
        memset((void*)page, 0, length);

    if(!(mask & __GFP_NO_INCREMENT)) {
        struct_page = physical2page(V2P(page));
        if(NULL == struct_page) {
            KPANIC("physical2page() returned NULL for allocated page: 0x%x",
                V2P(page));
        }

        // Lazy variable reuse
        order = length / PAGE_SIZE;
        while(order--) {
            atomic_inc(&struct_page->ref_count);
            struct_page++;
        }
    }

    return V2P(page);
}

// TODO: Improve this code. Looks like trash
// TODO: Coalesce the blocks
// TODO: Should the coalesce-ing be done in a thread? Prevents getting rid of single
//         page blocks constantly
// TODO: Check 'struct page' for page and only free if value == 0
void free_page(phys_addr_t addr)
{
    pmm_zone_t zone;
    bitmap_t *bitmap;
    size_t order, bit;
    char val;

    // Get the zone the address belongs to
    addr = ALIGN_PAGE(addr);
    if(addr < PMM_MEMORY_BOUND_DMA)
        zone = PMM_ZONE_DMA;
    else if(addr < PMM_MEMORY_BOUND_NORMAL)
        zone = PMM_ZONE_NORMAL;
    else
        zone = PMM_ZONE_HIGH;
    
    bitmap = &ram.zones[zone].split_blocks;

    // Start at max order, check if split
    order  = ram.zones[zone].max_order;
    bit    = BUDDY_INDEX_IN_LEVEL(addr, ram.zones[zone].base_addr, order);
    if(!bitmap_get(bitmap, bit, &val))
        goto err_checking_bitmap;
    // Abuse unsigned integer overflow to check when order should wrap negative
    while(order && 1 == val) {
        if(!bitmap_get(bitmap, bit, &val))
            goto err_checking_bitmap;
        order--;
        bit = BUDDY_INDEX_IN_LEVEL(addr, ram.zones[zone].base_addr, order);
    }

    if(order > ram.zones[zone].max_order) {
        return;
    }
    
    list_init((list_entry_t)P2V(addr));
    list_append(&ram.zones[zone].free_list[order], (list_entry_t)P2V(addr));
    ram.used -= PAGE_SIZE;

    return;

err_checking_bitmap:
    klog("Error getting bit from split bitmap!");
    return;
}

struct page* physical2page(phys_addr_t addr)
{
    struct page *ret;

    if(PMM_INVALID_PAGE == addr)
        return NULL;

    addr = ALIGN_PAGE(addr);

    ret = NULL;
    if(addr < ram.zones[PMM_ZONE_NORMAL].base_addr) {
        if(addr < ram.zones[PMM_ZONE_DMA].length) {
            ret = &(ram.first_struct_page[addr / PAGE_SIZE]);
        }
    } else if(addr < ram.zones[PMM_ZONE_HIGH].base_addr) {
        addr -= ram.zones[PMM_ZONE_NORMAL].base_addr;
        if(addr < ram.zones[PMM_ZONE_NORMAL].length) {
            ret = &(ram.first_struct_page[addr / PAGE_SIZE]);
        }
    } else {
        addr -= ram.zones[PMM_ZONE_HIGH].base_addr;
        if(addr < ram.zones[PMM_ZONE_HIGH].length) {
            ret = &(ram.first_struct_page[addr / PAGE_SIZE]);
        }
    }

    return ret;
}

phys_addr_t page2physical(struct page* page)
{
    size_t pfn_index;
    
    if(page < ram.first_struct_page)
        return PMM_INVALID_PAGE;

    pfn_index = (uintptr_t)page - (uintptr_t)ram.first_struct_page;
    pfn_index /= sizeof(struct page);

    if(pfn_index < ram.zones[PMM_ZONE_DMA].length / PAGE_SIZE)
        return ram.zones[PMM_ZONE_DMA].base_addr + pfn_index * PAGE_SIZE;
    pfn_index -= ram.zones[PMM_ZONE_DMA].length / PAGE_SIZE;

    if(pfn_index < ram.zones[PMM_ZONE_NORMAL].length / PAGE_SIZE)
        return ram.zones[PMM_ZONE_NORMAL].base_addr + pfn_index * PAGE_SIZE;
    pfn_index -= ram.zones[PMM_ZONE_NORMAL].length / PAGE_SIZE;

    if(pfn_index < ram.zones[PMM_ZONE_HIGH].length / PAGE_SIZE)
        return ram.zones[PMM_ZONE_HIGH].base_addr + pfn_index * PAGE_SIZE;

    return PMM_INVALID_PAGE;
}

static bool setup_buddy_allocator(multiboot_header_t mboot_header)
{
    struct multiboot_mmap_tag *mmap_tag;
    struct multiboot_mmap_entry_tag *mmap_entry_tag;
    size_t num_entries;

    mmap_tag = (struct multiboot_mmap_tag*)multiboot_get_tag(mboot_header,
        MULTIBOOT_TYPE_MEMORY_MAP);
    if(!mmap_tag) {
        return false;
    }

    // Sanity checks
    if(MULTIBOOT_MMAP_VERSION != mmap_tag->entry_version) {
        return false;
    }
    if(sizeof(*mmap_entry_tag) != mmap_tag->entry_size) {
        return false;
    }

    // Add regions to free-lists in buddy allocator for each zone
    num_entries = (mmap_tag->header.size - sizeof(*mmap_tag)) /
        sizeof(*mmap_entry_tag);
    mmap_entry_tag = &(mmap_tag->entries[0]);
    while(num_entries--) {
        ram.total += mmap_entry_tag->length;
        
        switch(mmap_entry_tag->type)
        {
        case MULTIBOOT_MMAP_TYPE_NVS:
            // TODO: Do anything special here for nvSRAM??
        case MULTIBOOT_MMAP_TYPE_AVAILABLE:
            ram.total_available += mmap_entry_tag->length;
            insert_ram_region(mmap_entry_tag->base_addr, mmap_entry_tag->length);
            break;
        default:
            break;
        }
        mmap_entry_tag++;
    }

    if(!create_split_bitmap()) return false;

    return true;
}

static void insert_ram_region(uintptr_t base, uint64_t length)
{
    if(base < PMM_MEMORY_BOUND_DMA) {
        if(base + length < PMM_MEMORY_BOUND_DMA) {
            insert_region(base, length, PMM_ZONE_DMA);
        } else {
            insert_region(base, PMM_MEMORY_BOUND_DMA - base, PMM_ZONE_DMA);
            length -= (PMM_MEMORY_BOUND_DMA - base);
            base    = PMM_MEMORY_BOUND_DMA;
        }
    }

    if(base >= PMM_MEMORY_BOUND_DMA && base < PMM_MEMORY_BOUND_NORMAL) {
        if(base + length < PMM_MEMORY_BOUND_NORMAL) {
            insert_region(base, length, PMM_ZONE_NORMAL);
        } else {
            insert_region(base, PMM_MEMORY_BOUND_NORMAL - base, PMM_ZONE_NORMAL);
            length -= (PMM_MEMORY_BOUND_NORMAL - base);
            base    = PMM_MEMORY_BOUND_NORMAL;
        }
    }

    if(base >= PMM_MEMORY_BOUND_NORMAL) {
        insert_region(base, length, PMM_ZONE_HIGH);
    }
}

static void insert_region(uintptr_t base, uint64_t length, pmm_zone_t zone)
{
    size_t num_pages, order;

    if(ram.zones[zone].base_addr > base) {
        ram.zones[zone].base_addr = base;
    }
    ram.zones[zone].length += length;

    num_pages = length / PAGE_SIZE;
    while(num_pages) {
        order = get_free_list_index_no_round(num_pages);
        if(order > PMM_FREE_LIST_MAX_ORDER) {
            order = PMM_FREE_LIST_MAX_ORDER;
        }
        if(order > ram.zones[zone].max_order) {
            ram.zones[zone].max_order = order;
        }
        list_init((list_entry_t)P2V(base));
        list_append(&ram.zones[zone].free_list[order], P2V(base));
        
        // TODO: Can num_pages ever become negative/interger overflow???
        num_pages -= (1 << order);
        base      += BUDDY_GET_SIZE(order);
    }
}

static bool create_split_bitmap()
{
    list_entry_t *first_free;
    size_t bitmap_size, bitmap_page_order;
    char *page;

    bitmap_size = 0;
    for(pmm_zone_t zone = PMM_ZONE_DMA; zone < PMM_ZONE_NUM; zone++) {
        bitmap_size += ALIGN_PTR((1 << ram.zones[zone].max_order) / 8);
    }
    ram.used += bitmap_size;

    bitmap_page_order = get_free_list_index(
        ALIGN_PAGE(bitmap_size) / PAGE_SIZE);
    
    if(!internal_alloc(bitmap_page_order, PMM_ZONE_NORMAL, false))
        return false;

    // If we are here we found page(s) that can store the bitmap
    first_free = &ram.zones[PMM_ZONE_NORMAL].free_list[bitmap_page_order];
    page = (char*)*first_free;
    *first_free = (*first_free)->next;

    // Initialize the bitmap for each zone
    buddy_bitmap_init(page);
    return true;
}

static void buddy_bitmap_init(char *page)
{
    size_t size, bit;

    for(pmm_zone_t zone = PMM_ZONE_DMA; zone < PMM_ZONE_NUM; zone++) {
        size = ALIGN_PTR((1 << ram.zones[zone].max_order));
        bitmap_init(&ram.zones[zone].split_blocks, page, size);

        // Because regions where added w/o marking a split value, that needs
        //  to be updated now
        for(size_t order = ram.zones[zone].max_order;
            order <= PMM_FREE_LIST_MAX_ORDER; order--) {
            // For each element already in the free-list
            list_foreach(ram.zones[zone].free_list[order], list) {
                // Split it's parent and up
                for(size_t tmp_order = order + 1;
                    tmp_order <= PMM_FREE_LIST_MAX_ORDER; tmp_order++) {
                    bit = BUDDY_INDEX_IN_LEVEL(V2P_KERNEL_BASED(list),
                        ram.zones[zone].base_addr, tmp_order);
                    bitmap_set(&ram.zones[zone].split_blocks, bit);
                }
            }
        }

        page += size;
    }
}

static inline void initialize_zone(struct pmm_zone *zone)
{
    memset(zone, 0, sizeof(*zone));
    zone->base_addr = PMM_INVALID_PAGE;
}

static bool buddy_split_block(list_entry_t block,
    size_t order, pmm_zone_t zone)
{
    if(!internal_split_block(block, order, zone)) {
        return false;
    }

    bitmap_toggle(&ram.zones[zone].split_blocks,
        BUDDY_INDEX_IN_LEVEL(V2P(block), ram.zones[zone].base_addr, order), NULL);

    return true;
}

static bool internal_split_block(list_entry_t block,
    size_t order, pmm_zone_t zone)
{
    list_entry_t *first_free;
    list_entry_t sub_block;

    if(0 == order || order > PMM_FREE_LIST_MAX_ORDER)
        return false;

    // Ensure that the block can actually belong to the zone/order pair
    first_free = &ram.zones[zone].free_list[order];
    if(NULL == *first_free)
        return false;

    // Update head of free_list if needed
    if(*first_free == block) {
        *first_free = block->next;
    }

    sub_block = block;
    list_remove(&block);

    order--;
    first_free = &ram.zones[zone].free_list[order];

    list_init(sub_block);
    list_append(first_free, sub_block);
    
    sub_block = ((list_entry_t)(((char*)sub_block) + BUDDY_GET_SIZE(order)));
    list_init(sub_block);
    list_append(first_free, sub_block);

    return true;
}

// TODO: Possibly macro this
static bool internal_alloc(size_t order, pmm_zone_t zone, bool use_bitmap)
{
    size_t curr_order;

    // Search for region of physical RAM that can hold the bitmap
    if(NULL == ram.zones[zone].free_list[order]) {
        // Need to split blocks to populate this!
        curr_order = order + 1;
        while(curr_order <= PMM_FREE_LIST_MAX_ORDER &&
            curr_order > order) {
            if(NULL != ram.zones[zone].free_list[curr_order]) {
                if(false == use_bitmap) {
                    internal_split_block(
                        ram.zones[zone].free_list[curr_order],
                        curr_order, zone);
                } else {
                    buddy_split_block(
                        ram.zones[zone].free_list[curr_order],
                        curr_order, zone);
                }
                curr_order--;
            } else {
                curr_order++;
            }
        }
        
        if(curr_order != order) {
            return false;
        }
    }

    return true;
}

static bool setup_struct_pages()
{
    size_t num_struct_pages_needed;
    phys_addr_t base_addr;

    num_struct_pages_needed = ALIGN_PAGE(ram.zones[PMM_ZONE_DMA].length);
    num_struct_pages_needed += ALIGN_PAGE(ram.zones[PMM_ZONE_NORMAL].length);
    num_struct_pages_needed += ALIGN_PAGE(ram.zones[PMM_ZONE_HIGH].length);
    num_struct_pages_needed /= PAGE_SIZE;

    base_addr = get_pages(
        num_struct_pages_needed * sizeof(struct page),
        __GPF_ZERO | __GFP_NO_INCREMENT);
    if(PMM_INVALID_PAGE == base_addr)
        return false;

    ram.first_struct_page = (struct page*)P2V(base_addr);

    return true;
}

static inline size_t get_free_list_index(size_t num_pages)
{
	size_t order;

    if(0 == num_pages) return (size_t)(-1);

	order = 0;
	while(0x1 != (num_pages >> order)) order++;

    // Round-up
    if(num_pages > (size_t)(1 << order)) order++;

	return order;
}

static inline size_t get_free_list_index_no_round(size_t num_pages)
{
	size_t order;

	order = 0;
	while(0x1 != (num_pages >> order)) order++;

	return order;
}