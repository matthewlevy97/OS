#pragma once

#ifndef __ASSEMBLER__

#include <stdbool.h>
#include <stdint.h>

#ifdef ARCH_x86_64
    typedef uint64_t interrupt_flag_t;

    #define interrupt_disable()   \
        do { __asm__ volatile ("cli"); } while(0);
    #define interrupt_enable()     \
        do { __asm__ volatile ("sti"); } while(0);
#endif

bool check_interrupts_enabled(void);
interrupt_flag_t interrupt_disable_flag(void);
void interrupt_enable_flag(interrupt_flag_t flags);

#define disable_interrupts_for                                              \
    for(interrupt_flag_t flags = interrupt_disable_flag(), __run_once = 1;  \
        __run_once; interrupt_enable_flag(flags), __run_once = 0)

#endif