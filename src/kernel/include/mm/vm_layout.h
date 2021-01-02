#pragma once

#include <common.h>

/**
 * Memory Layout (Userland)
 *  Code
 *  Heap
 *  Stack               0x7FF000000000  -> 0x7FFFFFFFFFFF   [64 GB]
 */
#define USER_STACK_START               (0x7FF000000000ULL)
#define USER_STACK_END                 (USER_STACK_START + (64 * GB) - 1)

/**
 * Memory Layout (Kernel):
 *  Code                0 (BASE)        ->  0x3FFFFFFF      [1    GB]
 *      KOutput Buffer  0               ->  0x1000          [4    Kb]
 *      Code            0x1000          ->  0x3FFFFFFF      [...]
 *  Heap                0x40000000      ->  0x1003FFFFFFF   [1    TB]
 *      KEternal        0x40000000      ->  0x7FFFFFFF      [1    GB]
 *      KPhysical Pages 0x80000000      ->  0x107FFFFFFF    [64   GB]
 *      KSlab Allocator 0x1080000000    ->  0x117FFFFFFF    [4    GB]
 *      KNormal Kmalloc 0x1180000000    ->  0x13FFFFFFFF    [10   GB]
 *      KPhys Page Tbl  0x1400000000    ->  0x1003FFFFFFF   [945  GB]
 *  Memory Map Region   0x10040000000   ->  0x5003FFFFFFF   [4    TB]
 *      KKernel Modules 0x10040000000   ->  0x2003FFFFFFF   [1    TB]
 *  Kernel Stack        0x50040000000   ->  0x6003FFFFFFF   [1    TB]
 */

/* General Offsets */
#define HEAP_OFFSET                   (1 * GB)
#define MEM_MAP_OFFSET                (HEAP_OFFSET + (1 * TB))
#define STACK_OFFSET                  (MEM_MAP_OFFSET + (4 * TB))

/* Kernel Offsets */
#define KERNEL_OFFSET                 (0xFFFF800000000000ULL)
#define KHEAP_OFFSET                  (KERNEL_OFFSET + HEAP_OFFSET)
#define KMEM_MAP_OFFSET               (KERNEL_OFFSET + MEM_MAP_OFFSET)
#define KSTACK_OFFSET                 (KERNEL_OFFSET + STACK_OFFSET)

#define KHEAP_END                     (KMEM_MAP_OFFSET - 1)
#define KMEM_MAP_END                  (KMEM_MAP_OFFSET + (4 * TB) - 1)
#define KSTACK_END                    (KSTACK_OFFSET + (1 * TB) - 1)

/* Kernel Heap Layout */
#define KHEAP_ETERNAL_OFFSET          (KHEAP_OFFSET)
#define KHEAP_PHYSICAL_PAGE_MAPPING   (KHEAP_ETERNAL_OFFSET        + (1   * GB))
#define KHEAP_SLAB_OFFSET             (KHEAP_PHYSICAL_PAGE_MAPPING + (64  * GB))
#define KHEAP_NORMAL_OFFSET           (KHEAP_SLAB_OFFSET           + (4   * GB))
#define KHEAP_PHYSICAL_PAGE_TABLE     (KHEAP_NORMAL_OFFSET         + (10  * GB))
#define KHEAP_END_OFFSET              (KHEAP_PHYSICAL_PAGE_TABLE   + (945 * GB))

#define KHEAP_ETERNAL_END                 (KHEAP_PHYSICAL_PAGE_MAPPING - 1)
#define KHEAP_PHYSICAL_PAGE_MAPPING_END   (KHEAP_SLAB_OFFSET           - 1)
#define KHEAP_SLAB_END                    (KHEAP_NORMAL_OFFSET         - 1)
#define KHEAP_NORMAL_END                  (KHEAP_PHYSICAL_PAGE_TABLE   - 1)
#define KHEAP_PHYSICAL_PAGE_TABLE_END     (KHEAP_END_OFFSET            - 1)

/* Kernel Mem Map Layout */
#define KMODULE_OFFSET                (KMEM_MAP_OFFSET)
#define KMEM_MAP_END_OFFSET           (KMODULE_OFFSET              + (1 * TB))

#define KMODULE_END                   (KMEM_MAP_END_OFFSET         - 1)