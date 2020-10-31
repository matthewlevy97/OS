#include <mm/kmalloc.h>
#include <mm/vm_layout.h>
#include <process/lock.h>

#include <stdint.h>

static uintptr_t base_addr = (uintptr_t)KHEAP_ETERNAL_OFFSET;
static size_t bytes_left   = (size_t)(KHEAP_ETERNAL_END - KHEAP_ETERNAL_OFFSET);

static spinlock_t lock = SPINLOCK_INIT;

void *kmalloc_eternal(size_t size, size_t align, gfp_t flags)
{
    void *ptr;
    size_t tmp_size;

    ptr      = NULL;
    tmp_size = size;
    if((base_addr % align) != 0) {
        tmp_size += (align - (base_addr % align));
    }

    spin_lock_acquire(&lock);

    if(tmp_size > bytes_left) {
        goto exit;
    }

    ptr         = (void*)(base_addr + (tmp_size - size));
    base_addr  += tmp_size;
    bytes_left -= tmp_size;

exit:
    spin_lock_release(&lock);
    return ptr;
}