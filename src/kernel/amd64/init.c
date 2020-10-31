#include <amd64/asm/segments.h>
#include <amd64/gdt.h>
#include <amd64/idt.h>
#include <amd64/init.h>
#include <devices/8254PIT/pit.h>
#include <devices/8259PIC/pic.h>
#include <string.h>

extern registers_t *task_scheduler_wrapper(registers_t *regs);

static struct cpu_local_data base_cpu_data = {
	.gdt_ptr  = {
		.size = GDT_SIZE - 1
	},
	.gdt_page = {
		.gdt = {
			[GDT_ENTRY_KERNEL32_CS]         = \
				GDT_ENTRY_INIT((uint64_t)(SEGMENT_PRESENT | SEGMENT_DB | SEGMENT_CODE), 0, 0xFFFFF),
			[GDT_ENTRY_KERNEL_CS]           = \
				GDT_ENTRY_INIT((uint64_t)(SEGMENT_PRESENT | SEGMENT_LONG_MODE | SEGMENT_CODE), 0, 0xFFFFF),
			[GDT_ENTRY_KERNEL_DS]           = \
				GDT_ENTRY_INIT((uint64_t)(SEGMENT_PRESENT | SEGMENT_DB | SEGMENT_DATA), 0, 0xFFFFF),
			[GDT_ENTRY_DEFAULT_USER32_CS]   = \
				GDT_ENTRY_INIT((uint64_t)(SEGMENT_PRESENT | SEGMENT_DB | SEGMENT_CODE | SEGMENT_USER), 0, 0xFFFFF),
			[GDT_ENTRY_DEFAULT_USER_DS]	    = \
				GDT_ENTRY_INIT((uint64_t)(SEGMENT_PRESENT | SEGMENT_DB | SEGMENT_DATA | SEGMENT_USER), 0, 0xFFFFF),
			[GDT_ENTRY_DEFAULT_USER_CS]	    = \
				GDT_ENTRY_INIT((uint64_t)(SEGMENT_PRESENT | SEGMENT_LONG_MODE | SEGMENT_CODE | SEGMENT_USER), 0, 0xFFFFF),
			[GDT_ENTRY_TSS]	    = \
				TSS_ENTRY_INIT()
		}
	}
};

struct cpu_local_data *amd64_init_core(struct cpu_local_data *cpu)
{
	if(NULL == cpu)
		cpu = &base_cpu_data;
	else
		memcpy(cpu, &base_cpu_data, sizeof(*cpu));

    gdt_init(cpu);
    tss_init(cpu);
	idt_init(cpu);
	
	pic_init();

	pit_init();
	pic_enable_interrupt(PIC_INTERNAL_TIMER);
	register_interrupt_handler(IRQ_BASE + PIC_TIMER_IRQ, task_scheduler_wrapper);

	return cpu;
}