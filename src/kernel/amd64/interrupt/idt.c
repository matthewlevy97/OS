#include <amd64/asm/segments.h>
#include <amd64/cpu.h>
#include <amd64/idt.h>
#include <common.h>
#include <string.h>

/* Located in amd64/interrupt/isr.S.py */
extern void idt_load(void *idtr);
extern uintptr_t isr_table[];

/* IDT Structures */
static struct idt_t {
    uint16_t offset_low;
    uint16_t cs;	// Code selector
    uint8_t  ist;	// Interrupt Stack Table
    uint8_t  flags;
    uint16_t offset_middle;
    uint32_t offset_high;
    uint32_t _;
} __packed idt[IDT_NUMBER_INTERRUPTS];
static struct {
    uint16_t  size;
    uintptr_t addr;
} __packed idtr;

/* Handler functions */
static int_handler_t idt_handlers[IDT_NUMBER_INTERRUPTS];

/* Static Function Definitions */
static void set_interrupt_gate(uint32_t, uintptr_t, uint16_t, uint8_t, uint8_t);

void idt_init(struct cpu_local_data *cpu)
{
    memset(idt_handlers, 0, sizeof(idt_handlers));

    for(int i = 0; i < IDT_NUMBER_INTERRUPTS; i++) {
        set_interrupt_gate(i, isr_table[i], __KERNEL_CS, 0,
            IDT_PRESENT | IDT_DPL0 | IDT_INTERRUPT_GATE);
    }
    
    idtr.size = sizeof(idt) - 1;
    idtr.addr = (uintptr_t)idt;

    idt_load(&idtr);
}

void register_interrupt_handler(uint32_t interrupt_number, int_handler_t handler)
{
    idt_handlers[interrupt_number] = handler;
}
void deregister_interrupt_handler(uint32_t interrupt_number)
{
    idt_handlers[interrupt_number] = NULL;
}

registers_t * interrupt_handler(registers_t *regs)
{
    if(idt_handlers[regs->int_no])
        return idt_handlers[regs->int_no](regs);
    
    return regs;
}

static void set_interrupt_gate(uint32_t num, uintptr_t isr, uint16_t cs,
    uint8_t ist, uint8_t flags)
{
    idt[num].offset_low    = isr         & 0xFFFF;
    idt[num].offset_middle = (isr >> 16) & 0xFFFF;
    idt[num].offset_high   = (isr >> 32) & 0xFFFFFFFFULL;
    idt[num].cs            = cs;
    idt[num].ist           = ist;
    idt[num].flags         = flags;
}