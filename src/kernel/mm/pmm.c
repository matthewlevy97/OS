#include <mm/kmalloc.h>
#include <mm/pmm.h>
#include <mm/paging.h>
#include <amd64/paging.h>
#include <common.h>
#include <string.h>
#include <klog.h>

/**
 * TODO:
 *  Keep free lists sorted by base address
 *      Makes checking for buddy a bit faster
 *  Add coalesce-ing code
 *  Code smells bad, might want to go back and improve at a later date
 *  Create "struct page"'s for all possible pages. Figure out best way to do this
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

    if(!setup_buddy_allocator(mboot_header)) {
        return false;
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
size_t pmm_used()
{
    return ram.used;
}

vm_addr_t get_free_page(pmm_gpf_t mask)
{
    list_entry_t *first_free;
    vm_addr_t page;
    pmm_zone_t zone;

    if(mask & __GFP_DMA)
        zone = PMM_ZONE_DMA;
    else if(mask & __GFP_HIGH)
        zone = PMM_ZONE_HIGH;
    else
        zone = PMM_ZONE_NORMAL;

    first_free = &ram.zones[zone].free_list[0];
    if(NULL == *first_free) {
        if(!internal_alloc(0, zone, true))
            return PMM_INVALID_PAGE;
    }

    // If we are here we found page(s) that can store the bitmap
    page = (uintptr_t)*first_free;
    *first_free = (*first_free)->next;

    ram.used += PAGE_SIZE;

    if(mask & __GPF_ZERO)
        memset((void*)page, 0, PAGE_SIZE);

    return page;
}

// TODO: Improve this code. Looks like trash
// TODO: Coalesce the blocks
// TODO: Should the coalesce-ing be done in a thread? Prevents getting rid of single
//         page blocks constantly
void free_page(vm_addr_t addr)
{
    pmm_zone_t zone;
    bitmap_t *bitmap;
    size_t order, bit;
    char val;

    // Get the zone the address belongs to
    addr = ALIGN_PAGE(addr);
    if(V2P(addr) < PMM_MEMORY_BOUND_DMA)
        zone = PMM_ZONE_DMA;
    else if(V2P(addr) < PMM_MEMORY_BOUND_NORMAL)
        zone = PMM_ZONE_NORMAL;
    else
        zone = PMM_ZONE_HIGH;
    
    bitmap = &ram.zones[zone].split_blocks;

    // Start at max order, check if split
    order  = ram.zones[zone].max_order;
    bit    = BUDDY_INDEX_IN_LEVEL(V2P(addr), ram.zones[zone].base_addr, order);
    if(!bitmap_get(bitmap, bit, &val))
        goto err_checking_bitmap;
    // Abuse unsigned integer overflow to check when order should wrap negative
    while(order && 1 == val) {
        if(!bitmap_get(bitmap, bit, &val))
            goto err_checking_bitmap;
        order--;
        bit = BUDDY_INDEX_IN_LEVEL(V2P(addr), ram.zones[zone].base_addr, order);
    }

    if(order > ram.zones[zone].max_order) {
        return;
    }
    
    list_init((list_entry_t)addr);
    list_append(&ram.zones[zone].free_list[order], (list_entry_t)addr);
    ram.used -= PAGE_SIZE;

    return;

err_checking_bitmap:
    klog("Error getting bit from split bitmap!");
    return;
}

static bool setup_buddy_allocator(multiboot_header_t mboot_header)
{
    struct multiboot_mmap_tag *mmap_tag;
    struct multiboot_mmap_entry_tag *mmap_entry_tag;
    uintptr_t critical_start, critical_end, base, bound;
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

    // Critical region is kernel + multiboot + loaded modules
    critical_start = V2P(&_kernel_start);
    critical_end   = ALIGN_PAGE((uintptr_t)&_kernel_end);
    critical_end  += ALIGN_PAGE(mboot_header->size);
    critical_end   = V2P(critical_end);
    // TODO: Add modules to critical end
    ram.used += critical_end - critical_start;

    // Add regions to free-lists in buddy allocator for each zone
    num_entries = (mmap_tag->header.size - sizeof(*mmap_tag)) /
        sizeof(*mmap_entry_tag);
    mmap_entry_tag = &(mmap_tag->entries[0]);
    while(num_entries--) {
        base  = mmap_entry_tag->base_addr;
        bound = mmap_entry_tag->base_addr + mmap_entry_tag->length;
        ram.total += mmap_entry_tag->length;
        switch(mmap_entry_tag->type)
        {
        case MULTIBOOT_MMAP_TYPE_NVS:
            // TODO: Do anything special here for nvSRAM??
        case MULTIBOOT_MMAP_TYPE_AVAILABLE:
            ram.total_available += mmap_entry_tag->length;

            // Check to see if overlaps critical regions: multiboot, kernel, etc
            
            if(base > critical_end || bound < critical_start) {
                insert_ram_region(base, mmap_entry_tag->length);
            } else {
                // Need to split around critical region
                insert_ram_region(base, critical_start - base);
                insert_ram_region(critical_end, bound - critical_end);
            }
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
                    bit = BUDDY_INDEX_IN_LEVEL(V2P(list),
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