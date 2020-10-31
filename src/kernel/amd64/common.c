#include <amd64/cpu.h>
#include <amd64/asm/segments.h>
#include <devices/8259PIC/pic.h>
#include <process/context.h>
#include <process/scheduler.h>
#include <klog.h>

void arch_dump_cpu_state()
{
	
}

void arch_save_context(struct process *process)
{
    // TODO: Save context (does not include stack pointer and general registers)
}

extern void isr_common_return_path(void);
__no_return void arch_load_context(struct process *process)
{
    // TODO: Load context (does not include stack pointer and general registers)

    __asm__ volatile(
        "mov %0, %%cr3\n\t"
        "mov %1, %%rax\n\t"
        "mov %2, %%rdi\n\r"
        "call %%rdi"
        :
        : "r" (process->vm_map->page_dir), "r" (process->kernel_stack_pointer),
          "r" (&isr_common_return_path)
        : "rax", "rdi");
    
    __unreachable;
}

void arch_prepare_stack(struct process *process, phys_addr_t physical_address)
{
    registers_t* regs;

    regs = (registers_t*)physical_address;
    regs->rip    = (uintptr_t)process->entry;
    regs->cs     = __KERNEL_CS;
    regs->rflags = X86_EFLAGS_STANDARD;
    regs->rsp    = ALIGN_PAGE(process->kernel_stack_pointer) - PAGE_SIZE;
    regs->ss     = __KERNEL_DS;
}

registers_t *task_scheduler_wrapper(registers_t *regs)
{
    regs = pic_acknowledge_interrupt(regs);

    schedule((uintptr_t)regs);

    return regs;
}