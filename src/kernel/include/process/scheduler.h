#pragma once

#include <process/process.h>

void scheduler_init(struct process*);

struct process *process_this();
struct process *get_process_by_pid(pid_t);

void scheduler_add(struct process*);
void scheduler_remove(struct process*);

void scheduler_set_priority(struct process*, enum process_priority);

void schedule(uintptr_t stack_ptr);