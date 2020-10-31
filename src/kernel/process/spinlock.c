#include <process/lock.h>
#include <process/scheduler.h>
#include <sys/types.h>
#include <klog.h>

static void lock_assert(spinlock_t*);

void spin_lock_init(spinlock_t *lock)
{
    lock->locked = false;
    lock->holder = TYPES_INVALID_PID;
}

void spin_lock_acquire(spinlock_t *lock)
{
    // TODO: Should replace with arch specific "halt" instruction
    while(!__sync_bool_compare_and_swap(&(lock->locked), 0, 1))
        __asm__ volatile("hlt");

    __sync_synchronize();

    if(NULL != process_this())
        lock->holder = process_this()->pid;
    else
        lock->holder = TYPES_INVALID_PID;
}

void spin_lock_release(spinlock_t *lock)
{
    lock_assert(lock);

    __sync_synchronize();

    lock->holder = TYPES_INVALID_PID;
    __atomic_store_n(&(lock->locked), 0, __ATOMIC_SEQ_CST);
}

bool spin_lock_try_acquire(spinlock_t *lock)
{
    if(__sync_bool_compare_and_swap(&(lock->locked), 0, 1)) {
        __sync_synchronize();
        
        if(NULL != process_this())
            lock->holder = process_this()->pid;
        else
            lock->holder = TYPES_INVALID_PID;
        
        return true;
    } else {
        return false;
    }
}

static void lock_assert(spinlock_t *lock)
{
    // TODO: Not sure how to handle this case
    if(NULL == process_this()) return;

    if(lock->holder != process_this()->pid && !lock->locked) {
        KPANIC("Thread(%d) holding lock isn't same as thread(%d) releasing!",
            lock->holder, process_this()->pid);
    }
}