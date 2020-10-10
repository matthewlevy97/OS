#include <amd64/paging.h>
#include <mm/paging.h>

#include <stdint.h>

static uint32_t get_p4_index(uintptr_t);
static uint32_t get_p3_index(uintptr_t);
static uint32_t get_p2_index(uintptr_t);
static uint32_t get_p1_index(uintptr_t);

bool paging_init()
{
    return true;
}

void paging_make_canonical(vm_addr_t *addr)
{
    if((*addr >> 47) & 0x1) {
        *addr |= (0xFFFFULL << 48);
    } else {
        *addr &= ~(0xFFFFULL << 48);
    }
}

static inline uint32_t get_p4_index(uintptr_t address)
{
    return (address >> 39) & 511;
}
static __inline uint32_t get_p3_index(uintptr_t address)
{
    return (address >> 30) & 511;
}
static __inline uint32_t get_p2_index(uintptr_t address)
{
    return (address >> 21) & 511;
}
static __inline uint32_t get_p1_index(uintptr_t address)
{
    return (address >> 12) & 511;
}