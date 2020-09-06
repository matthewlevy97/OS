#pragma once

#include <amd64/cpu.h>

void gdt_init(struct cpu_local_data *);
void tss_init(struct cpu_local_data *);

void gdt_reload(struct desc_ptr *);
void tss_install(void);