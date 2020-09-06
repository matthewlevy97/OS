#pragma once

#include <amd64/cpu.h>

#ifndef __ASSEMBLER__

#include <stdbool.h>
#include <stdint.h>

void isr_common(void);
void idt_load(void *);

bool check_interrupts_enabled(void);
uint64_t interrupt_disable_flag(void);
void interrupt_enable_flag(uint64_t rflags);

#define interrupt_disable()   \
    do { __asm__ volatile ("cli"); } while(0);
#define interrupt_enable()     \
    do { __asm__ volatile ("sti"); } while(0);

#define disable_interrupts_for                                      \
    for(uint64_t rflags = interrupt_disable_flag(), __run_once = 1; \
        __run_once; interrupt_enable_flag(rflags), __run_once = 0)

#endif