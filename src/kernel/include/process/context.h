#pragma once

#include <mm/paging.h>
#include <process/process.h>

struct cpu_local_data;

#ifdef ARCH_x86_64
#include <amd64/cpu.h>
#endif

extern void arch_save_context(struct process*);
extern void arch_load_context(struct process*);

extern void arch_prepare_stack(struct process*, phys_addr_t);
extern void arch_userland_trampoline(void);

extern struct cpu_local_data *arch_get_current_cpu();