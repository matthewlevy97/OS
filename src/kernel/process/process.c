#include <mm/kmalloc.h>
#include <mm/paging.h>
#include <mm/pmm.h>
#include <process/context.h>
#include <process/process.h>
#include <klog.h>
#include <string.h>

static pid_t generate_pid();

static bool generate_kernel_stack(struct process*);
static bool generate_user_stack(struct process*);

struct process *process_create(const char *name, void (*entry)(),
    enum process_create_flags flags)
{
    struct process *proc;

    proc = kmalloc(sizeof(*proc), GFP_KERNEL | GFP_ZERO);
    if(NULL == proc)
        goto err_proc;
    spin_lock_init(&proc->lock);

    proc->vm_map = paging_create_vmmap();
    if(NULL == proc->vm_map)
        goto err_proc;

    list_init(proc);
    
    proc->pid = generate_pid();
    strncpy(proc->name, name, sizeof(proc->name));
    
    proc->user     = flags & PROCESS_CREATE_USER;
    proc->state    = PROCESS_STATE_RUNNING;
    proc->priority = PROCESS_PRIORITY_NORMAL;

    if(false == proc->user)
        proc->entry = entry;
    else
        proc->entry = &arch_userland_trampoline;
    
    if(!generate_kernel_stack(proc))
        goto err_proc;
    if(!generate_user_stack(proc))
        goto err_proc;

    return proc;

err_proc:
    process_destroy(proc);
    return NULL;
}

struct process *process_duplicate(struct process *proc)
{
    // TODO: Process duplicate
    return NULL;
}

void process_destroy(struct process *proc)
{
    if(NULL == proc) return;

    spin_lock_acquire(&proc->lock);
    list_remove(&proc);

    if(proc->vm_map) {
        // TODO: Destroy page map / unmap everything
        //paging_destroy_vmmap(&proc->vm_map);
    }

    // For safety/security
    memset(proc, 0, sizeof(*proc));

    kfree(proc);
}

void process_push_kernel_stack(struct process *proc, void *ptr, size_t size)
{
    proc->kernel_stack_pointer -= size;
    memcpy((void*)proc->kernel_stack_pointer, ptr, size);
}

void process_push_user_stack(struct process *proc, void *ptr, size_t size)
{
    proc->user_stack_pointer -= size;
    memcpy((void*)proc->user_stack_pointer, ptr, size);
}

void process_map_program(struct process *proc)
{
    // TODO: This is not an implementation to load a program!!!
    uintptr_t val;

    paging_map(proc->vm_map, 0xDEADBEEF, PAGE_MAP_PRESENT|PAGE_MAP_RING3);
    memcpy(0xDEADBEEF, "\xeb\xfe", 2); // jmp $

    val = 0xCAFEBABE;
    process_push_user_stack(proc, &val, sizeof(val));

    proc->entry = 0xDEADBEEF;
}

static pid_t generate_pid()
{
    // TODO: Generate PIDs in a better manner
    static pid_t pid = 0;
    return pid++;
}

static bool generate_kernel_stack(struct process *proc)
{
    uintptr_t stack_base;
    phys_addr_t physical_page;

    stack_base = ALIGN_PAGE(KSTACK_END) - PAGE_SIZE;

#if defined(PROCESS_INITIAL_KSTACK_PAGES) && PROCESS_INITIAL_KSTACK_PAGES >= 2
    paging_map_range(proc->vm_map,
        stack_base - PAGE_SIZE * (PROCESS_INITIAL_KSTACK_PAGES - 1),
        PAGE_SIZE * (PROCESS_INITIAL_KSTACK_PAGES - 1),
        PAGE_MAP_PRESENT | PAGE_MAP_RW | PAGE_MAP_NX);
#endif

    // TODO: Add check to ensure get_free_page() was successful
    physical_page = get_free_page(__GPF_ZERO);
    paging_map2(proc->vm_map, stack_base, physical_page,
        PAGE_MAP_PRESENT | PAGE_MAP_RW);
    
    proc->kernel_stack_pointer = stack_base + PAGE_SIZE - sizeof(void*);
    proc->kernel_stack_base    = proc->kernel_stack_pointer;
    arch_prepare_stack(proc,
        (phys_addr_t)(P2V(physical_page) + PAGE_SIZE - sizeof(void*)));

    return true;
}

static bool generate_user_stack(struct process *proc)
{
    registers_t *regs;
    uintptr_t stack_base;
    phys_addr_t physical_page;

    // Just leave if we shouldn't create a userland stack
    if(false == proc->user)
        return true;

    stack_base = ALIGN_PAGE(USER_STACK_END) - PAGE_SIZE;

#if defined(PROCESS_INITIAL_STACK_PAGES) && PROCESS_INITIAL_STACK_PAGES >= 2
    paging_map_range(proc->vm_map,
        stack_base - PAGE_SIZE * (PROCESS_INITIAL_STACK_PAGES - 1),
        PAGE_SIZE * (PROCESS_INITIAL_STACK_PAGES - 1),
        PAGE_MAP_PRESENT | PAGE_MAP_RW | PAGE_MAP_NX | PAGE_MAP_RING3);
#endif

    physical_page = get_free_page(__GPF_ZERO);
    paging_map2(proc->vm_map, stack_base, physical_page,
        PAGE_MAP_PRESENT | PAGE_MAP_RW | PAGE_MAP_NX | PAGE_MAP_RING3);
    
    proc->user_stack_pointer = stack_base + PAGE_SIZE - sizeof(void*);

    return true;
}