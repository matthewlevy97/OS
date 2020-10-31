#pragma once

#include <sys/types.h>
#include <stdbool.h>

typedef struct {
    bool  locked;
    pid_t holder;
} spinlock_t;

#define SPINLOCK_INIT                   \
{                                       \
    .locked = false,                    \
    .holder = TYPES_INVALID_PID,        \
}

void spin_lock_init(spinlock_t*);

void spin_lock_acquire(spinlock_t*);
void spin_lock_release(spinlock_t*);

bool spin_lock_try_acquire(spinlock_t*);