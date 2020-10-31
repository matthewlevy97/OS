#include <mm/paging.h>
#include <process/context.h>
#include <process/scheduler.h>
#include <sys/types.h>
#include <klog.h>
#include <list.h>
#include <stddef.h>

static struct process *running_head[PROCESS_NUM_PRIORITIES];
static struct process *current_running;
static struct process *blocked_head;
static size_t priority_min, priority_max;

static struct process *search_for_next_process(struct process *head);
static struct process *scan_priority_list();
static void wake_blocked_tasks();

__no_return void scheduler_init(struct process *proc)
{
    for(int i = 0; i < PROCESS_NUM_PRIORITIES; i++) {
        running_head[i] = NULL;
    }
    blocked_head = NULL;

    running_head[PROCESS_PRIORITY_LOW] = proc;
    current_running = running_head[PROCESS_PRIORITY_LOW];

    priority_min = 0;
    priority_max = 0;

    arch_load_context(proc);

    __unreachable;
}

struct process *process_this()
{
    if(NULL == current_running)
        return NULL;
    return current_running;
}

struct process *get_process_by_pid(pid_t pid)
{
    if(NULL == running_head) return NULL;
    for(int i = 0; i < PROCESS_NUM_PRIORITIES; i++) {
        list_foreach(running_head[i], proc) {
            if(proc->pid == pid) return proc;
        }
    }

    if(NULL == blocked_head) return NULL;
    list_foreach(blocked_head, proc) {
        if(proc->pid == pid) return proc;
    }

    return NULL;
}

void scheduler_add(struct process *proc)
{
    if(proc->state != PROCESS_STATE_BLOCKED) {
        list_append(&running_head[proc->priority], proc);
    } else {
        list_append(&blocked_head, proc);
    }
}

void scheduler_remove(struct process *proc)
{
    if(running_head[proc->priority] == proc)
        running_head[proc->priority] = proc->next;
    
    list_remove(&proc);
}

void scheduler_set_priority(struct process *proc, enum process_priority priority)
{
    proc->priority = priority;

    scheduler_remove(proc);
    scheduler_add(proc);
}

void schedule(uintptr_t stack_ptr)
{
    struct process *next_proc;

    wake_blocked_tasks();

    if(NULL == current_running) return;
    
    current_running->kernel_stack_pointer = stack_ptr;
    arch_save_context(current_running);

    next_proc = search_for_next_process(current_running->next);
    if(NULL == next_proc) {
        next_proc = scan_priority_list();
    }

    if(next_proc)
        current_running = next_proc;

    arch_load_context(current_running);

    __unreachable;
}

static struct process *search_for_next_process(struct process *head)
{
    struct process *ret, *tmp;

    ret = NULL;
    list_foreach(head, proc) {
        if(proc->state == PROCESS_STATE_RUNNING) {
            ret   = proc;
            break;
        } else if(proc->state == PROCESS_STATE_BLOCKED) {
            tmp = proc;

            if(proc->prev != proc && proc != current_running) {
                proc = proc->prev;

                list_remove(&tmp);
                list_append(&blocked_head, tmp);
                klog("Moving process: %s\n", proc->name);
            }
        } else if(proc->state == PROCESS_STATE_DESTROY) {
            tmp = proc;

            // TODO: There is probably a better way to do this
            if(proc->prev != proc && proc != current_running) {
                proc = proc->prev;
                process_destroy(tmp);
            } else {
                // Only 1 process in running queue and it wants to destroy self
                KPANIC("Only running process wants to kill self!");
            }
        }
    }

    return ret;
}

static struct process *scan_priority_list()
{
    struct process *ret;

    ret = NULL;
    for(; priority_min <= priority_max && NULL == ret; priority_min++) {
        ret = search_for_next_process(running_head[priority_min]);
    }
    
    if(NULL == ret && priority_min >= priority_max) {
        priority_min = 0;
        priority_max = (priority_max + 1) % PROCESS_NUM_PRIORITIES;
    }

    return ret;
}

static void wake_blocked_tasks()
{
    // TODO: Wake any process that is sleeping but should be woken
}