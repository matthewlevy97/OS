#include <mm/kmalloc.h>
#include <mm/vm_layout.h>
#include <common.h>
#include <klog.h>
#include <stdint.h>

extern void *kmalloc_eternal(size_t size, size_t align, gfp_t flags);

void *kmalloc_a(size_t size, size_t align, gfp_t flags)
{
    if(flags & GFP_ETERNAL) {
        return kmalloc_eternal(size, align, flags);
    }
    return NULL;
}

void free(void *ptr)
{
    /* Determine region block belongs to */
    if((uintptr_t)ptr < KHEAP_OFFSET || (uintptr_t)ptr > KHEAP_END) {
        // ptr not in the heap!
        KPANIC("Cannot free ptr outside of heap: 0x%x\n", ptr);
        return;
    }

    if((uintptr_t)ptr < KHEAP_ETERNAL_END) {
    } else if((uintptr_t)ptr < KHEAP_PHYSICAL_PAGE_MAPPING_END) {
    } else if((uintptr_t)ptr < KHEAP_SLAB_END) {
    } else if((uintptr_t)ptr < KHEAP_NORMAL_END) {
    } else { // Implied: if(ptr < KHEAP_PHYSICAL_PAGE_TABLE_END)
    }
}