#include <devices/8259PIC/pic.h>

static void remap_interrupts();
static uint16_t read_mask();
static void write_mask(uint16_t);

bool pic_init()
{
    remap_interrupts();

    // Disable all interrupts except cascade to slave
    write_mask(PIC_MASTER_TO_SLAVE);

    return true;
}

bool pic_destroy()
{
    outb(PIC_SLAVE_BASE + PIC_DATA_OFFSET, 0xFF);
    outb(PIC_MASTER_BASE + PIC_DATA_OFFSET, 0xFF);

    return true;
}

void pic_enable_interrupt(enum pic_interrupts interrupt)
{
    uint16_t mask;

    mask = read_mask();
    mask |= interrupt;
    write_mask(mask);
}

void pic_disable_interrupt(enum pic_interrupts interrupt)
{
    uint16_t mask;

    mask = ~read_mask();
    mask &= ~((uint16_t)interrupt);
    write_mask(mask);
}

registers_t *pic_acknowledge_interrupt(registers_t *regs)
{
    if(regs->int_no >= IRQ_BASE && regs->int_no - IRQ_BASE >= 8)
        outb(PIC_SLAVE_BASE + PIC_COMMAND_OFFSET, PIC_END_OF_INTERRUPT);
    outb(PIC_MASTER_BASE + PIC_COMMAND_OFFSET, PIC_END_OF_INTERRUPT);

    return regs;
}

static void remap_interrupts()
{
    uint8_t master_mask, slave_mask;

    master_mask = inb(PIC_MASTER_BASE + PIC_DATA_OFFSET);
    slave_mask  = inb(PIC_SLAVE_BASE + PIC_DATA_OFFSET);

    // Start initialization sequence
    outb(PIC_MASTER_BASE + PIC_COMMAND_OFFSET, PIC_ICW1);
    outb(PIC_SLAVE_BASE + PIC_COMMAND_OFFSET, PIC_ICW1);

    outb(PIC_MASTER_BASE + PIC_DATA_OFFSET, IRQ_BASE);
    outb(PIC_SLAVE_BASE + PIC_DATA_OFFSET, IRQ_BASE + 8);

    outb(PIC_MASTER_BASE + PIC_DATA_OFFSET, PIC_ICW3_MASTER);
    outb(PIC_SLAVE_BASE + PIC_DATA_OFFSET, PIC_SLAVE_IDENTIFICATION_CODE);

    outb(PIC_MASTER_BASE + PIC_DATA_OFFSET, PIC_ICW4);
    outb(PIC_SLAVE_BASE + PIC_DATA_OFFSET, PIC_ICW4);

    outb(PIC_MASTER_BASE + PIC_DATA_OFFSET, master_mask);
    outb(PIC_SLAVE_BASE + PIC_DATA_OFFSET, slave_mask);
}

static __inline uint16_t read_mask()
{
    return ~((inb(PIC_SLAVE_BASE + PIC_DATA_OFFSET) << 8) |
        inb(PIC_MASTER_BASE + PIC_DATA_OFFSET));
}

static __inline void write_mask(uint16_t mask)
{
    mask = ~mask;
    outb(PIC_MASTER_BASE + PIC_DATA_OFFSET, mask & 0xFF);
	outb(PIC_SLAVE_BASE + PIC_DATA_OFFSET, (mask >> 8) & 0xFF);
}