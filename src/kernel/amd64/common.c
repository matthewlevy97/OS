#include <amd64/cpu.h>
#include <amd64/asm/segments.h>
#include <devices/8259PIC/pic.h>
#include <process/context.h>
#include <process/scheduler.h>
#include <string.h>
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
    struct cpu_local_data *cpu;

    // TODO: Load context (does not include stack pointer and general registers)

    interrupt_disable();

    cpu = arch_get_current_cpu();
    cpu->tss.rsp[0] = process->kernel_stack_base;

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
    registers_t regs;

    disable_interrupts_for {
        memset(&regs, 0x00, sizeof(regs));
        regs.rip    = (uintptr_t)process->entry;
        regs.cs     = __KERNEL_CS;
        regs.rflags = X86_EFLAGS_STANDARD;
        regs.rsp    = process->kernel_stack_pointer;
        regs.ss     = __KERNEL_DS;
        memcpy((char*)physical_address - sizeof(regs), &regs, sizeof(regs));

        process->kernel_stack_pointer -= sizeof(regs);
    }
}

__no_return void arch_userland_trampoline(void)
{
    struct process *proc;
    registers_t regs;
    
    interrupt_disable();

    proc = process_this();

    process_map_program(proc);

    // TODO: Confirm these are the correct segments
    regs.rip    = (uintptr_t)proc->entry;
    regs.cs     = __USER_CS;
    regs.rflags = X86_EFLAGS_STANDARD;
    regs.rsp    = proc->user_stack_pointer;
    regs.ss     = __USER_DS;
    proc->kernel_stack_pointer = (uintptr_t)&regs;

    arch_load_context(proc);
    
    __unreachable;
}

registers_t *task_scheduler_wrapper(registers_t *regs)
{
    regs = pic_acknowledge_interrupt(regs);

    schedule((uintptr_t)regs);

    return regs;
}