#pragma once

#include <debug.h>
#include <interrupt.h>
#include <stddef.h>

#define KPANIC(...)                                                         \
    do {                                                                    \
        interrupt_disable();                                                \
        klog("Kernel Panic!\n");                                            \
        klog(__VA_ARGS__);                                                  \
        debug_dump_cpu_state();                                             \
        while(1);                                                           \
    } while(0);

int klog_init(void *buffer, size_t length);

void klog(const char *fmt, ...);