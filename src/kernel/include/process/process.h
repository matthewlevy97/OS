#pragma once

#include <mm/paging.h>
#include <process/lock.h>
#include <sys/types.h>
#include <list.h>
#include <stdbool.h>
#include <stdint.h>

#define PROCESS_INITIAL_KSTACK_PAGES   (2)
#define PROCESS_INITIAL_STACK_PAGES    (2)
#define PROCESS_NAME_SIZE              (128)

enum process_priority {
    PROCESS_PRIORITY_HIGH    = 0,
    PROCESS_PRIORITY_NORMAL  = 1,
    PROCESS_PRIORITY_LOW     = 2,
    PROCESS_PRIORITY_IDLE    = 3,
    PROCESS_NUM_PRIORITIES   = 4
};

enum process_state {
    PROCESS_STATE_RUNNING,
    PROCESS_STATE_BLOCKED,
    PROCESS_STATE_DESTROY
};

enum process_create_flags {
    PROCESS_CREATE_USER        = 1 << 0,
};

struct process {
    ListEntry_HEAD

    pid_t pid;
    char name[PROCESS_NAME_SIZE];

    bool user;

    enum process_state state;
    enum process_priority priority;

    struct vm_map *vm_map;
    uintptr_t user_stack_pointer;
    uintptr_t kernel_stack_pointer, kernel_stack_base;
    
    spinlock_t lock;

    int exit_value;

    void (*entry)();
};

struct process *process_create(const char*, void (*entry)(),
    enum process_create_flags);
struct process *process_duplicate(struct process*);
void process_destroy(struct process*);

void process_push_kernel_stack(struct process*, void*, size_t);
void process_push_user_stack(struct process*, void*, size_t);

void process_map_program(struct process*);