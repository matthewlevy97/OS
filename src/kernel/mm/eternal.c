#include <mm/kmalloc.h>
#include <mm/vm_layout.h>

#include <stdint.h>

static uintptr_t base_addr  = (uintptr_t)KHEAP_ETERNAL_OFFSET;
static size_t bytes_left = (size_t)(KHEAP_ETERNAL_END - KHEAP_ETERNAL_OFFSET);

// TODO: Add spinlocks for this
void *kmalloc_eternal(size_t size, gfp_t flags)
{
    void *ptr;

    if(size > bytes_left) {
        return NULL;
    }

    ptr         = (void*)base_addr;
    base_addr  += size;
    bytes_left -= size;

    return ptr;
}