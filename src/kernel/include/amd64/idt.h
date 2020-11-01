#pragma once

#include <amd64/cpu.h>
#include <stdint.h>

#define IRQ_BASE                       (0x20)
#define IRQ_LENGTH                     (0xF)
#define IRQ_TO_ISR(x)                  ((x) + (IRQ_BASE))

#define IDT_NUMBER_INTERRUPTS          (256)

enum {
    IDT_PRESENT        = 1 << 7,

    IDT_DPL0           = 0x00, // RING-0
    IDT_DPL3           = 0x60, // Allow interrupt to be initiated when RING-3

    IDT_INTERRUPT_GATE = 0xE,
    IDT_TRAP_GATE      = 0xF,
};

typedef registers_t *(*int_handler_t)(registers_t *);

void idt_init(struct cpu_local_data *);

void register_interrupt_handler(uint32_t, int_handler_t);
void deregister_interrupt_handler(uint32_t);

registers_t * interrupt_handler(registers_t *);