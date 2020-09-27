#pragma once

#ifndef __ASSEMBLER__

#define __packed                       __attribute__((packed))
#define __align(bytes)                 __attribute__ ((aligned (bytes)))
#define __inline                       inline __attribute__((__always_inline__))

#define UNUSED(_)                      ((void)(_))

#include <stdint.h>
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