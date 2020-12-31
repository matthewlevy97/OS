#include <mm/kmalloc.h>
#include <mm/vm_layout.h>
#include <common.h>
#include <klog.h>
#include <list.h>
#include <stdint.h>
#include <string.h>

static struct kmalloc_preamble *first_free_block;

extern void *kmalloc_eternal(size_t, size_t, gfp_t);

static void *alloc(size_t, gfp_t);
static void free(void*);

bool kmalloc_init(struct vm_map *vm_map)
{
    first_free_block = (struct kmalloc_preamble*)KHEAP_NORMAL_OFFSET;
    paging_map(vm_map, KHEAP_NORMAL_OFFSET, KMALLOC_PAGE_MAP_FLAGS);

    list_init(first_free_block);
    first_free_block->blk_size = PAGE_SIZE - sizeof(*first_free_block);
    first_free_block->flags    = 0;

    return true;
}

void *kmalloc_a(size_t size, size_t align, gfp_t flags)
{
    void *ptr;
    if(flags & GFP_ETERNAL) {
        ptr = kmalloc_eternal(size, align, flags);
    } else {
        // TODO: Handle alignment
        ptr = alloc(size, flags);
    }

    if(NULL != ptr && flags & GFP_ZERO) {
        memset(ptr, 0, size);
    }

    return ptr;
}

void kfree(void *ptr)
{
    /* Determine region block belongs to */
    if((uintptr_t)ptr < KHEAP_OFFSET || (uintptr_t)ptr > KHEAP_END) {
        if(NULL == ptr) {
            KPANIC("Tried to free NULL pointer!");
            return;
        }

        // ptr not in the heap!
        KPANIC("Cannot free ptr outside of heap: 0x%x\n", ptr);
        return;
    }
    
    if((uintptr_t)ptr >= KHEAP_NORMAL_OFFSET &&
        (uintptr_t)ptr < KHEAP_NORMAL_END) {
        free(ptr);
    } else {
        KPANIC("Cannot free memory! [ptr = 0x%x]\n", ptr);
    }
}

static void *alloc(size_t size, gfp_t flags)
{
    struct kmalloc_preamble *ret_block;
    ret_block = NULL;
    
    list_foreach(first_free_block, block) {
        if(!(block->flags & KMALLOC_IN_USE) && block->blk_size >= size) {
            ret_block = block;
            ret_block->flags |= KMALLOC_IN_USE;

            // Should block be split??
            if(block->blk_size - size - sizeof(*block) >= KMALLOC_MIN_SPLIT_SIZE) {
                block = (struct kmalloc_preamble*)((char*)(&block->data) + size);

                list_insert_after(ret_block, block);

                // Update block sizes
                block->blk_size = ret_block->blk_size - size - sizeof(*block);
                ret_block->blk_size = size;

                block->flags = KMALLOC_INIT_FLAGS;
            }

            break;
        }
    }
    
    if(ret_block)
        return &ret_block->data;
    return NULL;
}

static void free(void *ptr)
{
    struct kmalloc_preamble *block, *tmp;

    block = ((struct kmalloc_preamble*)ptr) - 1;
    if(0 == (block->flags & KMALLOC_IN_USE)) {
        KPANIC("Double-free for pointer! [ptr = 0x%x]\n", ptr);
        return;
    }

    block->flags ^= KMALLOC_IN_USE;
    
    // Combine blocks!!!
    while(block->prev) {
        if((uintptr_t)block->prev->next != (uintptr_t)block) {
            KPANIC("[KMalloc] Previous block has broken chain: 0x%x 0x%x\n",
                block->next->prev, block);
            return;
        }

        tmp = (struct kmalloc_preamble*)(block->prev);
        if(0 == (tmp->flags & KMALLOC_IN_USE)) {
            tmp->next = block->next;
            if(block->next)
                block->next->prev = (struct _list_entry*)tmp;
            
            tmp->blk_size += sizeof(*block) + block->blk_size;
            block = tmp;
        } else {
            break;
        }
    }

    while(block->next) {
        if((uintptr_t)block->next->prev != (uintptr_t)block) {
            KPANIC("[KMalloc] Next block has broken chain: 0x%x 0x%x\n",
                block->next->prev, block);
            return;
        }

        tmp = (struct kmalloc_preamble*)(block->next);
        if(0 == (tmp->flags & KMALLOC_IN_USE)) {
            block->next = tmp->next;
            if(block->next)
                block->next->prev = (struct _list_entry*)block;
            block->blk_size += sizeof(*tmp) + tmp->blk_size;
        } else {
            break;
        }
    }
}