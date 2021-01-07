#pragma once

#include <process/scheduler.h>
#include <interrupt.h>
#include <stddef.h>

extern void arch_dump_cpu_state();

#define KPANIC(...)                                                            \
    do {                                                                       \
        interrupt_disable();                                                   \
        klog("Kernel Panic!\n");                                               \
        klog("File: %s, Func: %s, Line: %d\n", __FILE__, __func__, __LINE__);  \
        if(process_this())                                                     \
            klog("Process PID: %d\n", process_this()->pid);                    \
        klog(__VA_ARGS__);                                                     \
        arch_dump_cpu_state();                                                 \
        while(1);                                                              \
    } while(0);

int klog_init(void *buffer, size_t length);

void klog(const char *fmt, ...);