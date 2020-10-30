#pragma once

#ifndef __ASSEMBLER__

#include <stdint.h>

#define __packed                       __attribute__((packed))
#define __align(bytes)                 __attribute__ ((aligned (bytes)))
#define __inline                       inline __attribute__((__always_inline__))
#define __used                          __attribute__((__used__))

#define UNUSED(_)                      ((void)(_))

// Aligns Up
#define __ALIGN_MASK(x, mask)          (((x) + (mask)) & (~(mask)))
#define ALIGN_LOG2(x, y)               __ALIGN_MASK(x, ((__typeof__(x))(y))-1)
#define ALIGN_PTR(x)                   ALIGN_LOG2(x, sizeof(void*))
// Leave the definition of "PAGE_SIZE" to the calling code
#define ALIGN_PAGE(x)                  ALIGN_LOG2(x, PAGE_SIZE)

#define SIZE_T_SIZE                    ALIGN_LOG2(sizeof(size_t))

extern uintptr_t _kernel_start;
extern uintptr_t _kernel_text_end;
extern uintptr_t _kernel_data_end;
extern uintptr_t _kernel_end;

#endif

/* Data Sizes */
#define KB                             (0x00000000400ULL)
#define MB                             (0x00000100000ULL)
#define GB                             (0x00040000000ULL)
#define TB                             (0x10000000000ULL)