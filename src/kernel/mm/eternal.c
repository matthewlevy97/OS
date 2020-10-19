#include <mm/kmalloc.h>
#include <mm/vm_layout.h>

#include <stdint.h>

static uintptr_t base_addr  = (uintptr_t)KHEAP_ETERNAL_OFFSET;
static size_t bytes_left = (size_t)(KHEAP_ETERNAL_END - KHEAP_ETERNAL_OFFSET);

// TODO: Add spinlocks for this
void *kmalloc_eternal(size_t size, size_t align, gfp_t flags)
{
    void *ptr;
    size_t tmp_size;

    tmp_size = size;
    if((base_addr % align) != 0) {
        tmp_size += (align - (base_addr % align));
    }

    if(tmp_size > bytes_left) {
        return NULL;
    }

    ptr         = (void*)(base_addr + (tmp_size - size));
    base_addr  += tmp_size;
    bytes_left -= tmp_size;

    return ptr;
}