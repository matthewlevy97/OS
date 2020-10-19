#pragma once

#include <devices/devices.h>
#include <stdbool.h>

/**
 * Information Sheet:
 *  https://www.intel.com/content/dam/www/public/us/en/documents/datasheets/7-series-chipset-pch-datasheet.pdf
 */

#define PIC_MASTER_BASE                (0x20)
#define PIC_SLAVE_BASE                 (0xA0)

#define PIC_END_OF_INTERRUPT           (0x20)

#define PIC_COMMAND_OFFSET             (0x00)
#define PIC_DATA_OFFSET                (0x01)

#define PIC_ICW1                       (1 << 4)

#define PIC_ICW3_MASTER                (1 << 2)
#define PIC_SLAVE_IDENTIFICATION_CODE  (0x02)

#define PIC_ICW4                       (0x01)

enum pic_interrupts {
	PIC_INTERNAL_TIMER  = 1 << 0,
	PIC_KEYBOARD        = 1 << 1,
	PIC_MASTER_TO_SLAVE = 1 << 2,
	PIC_SERIAL_PORT_A   = 1 << 3,
	PIC_SERIAL_PORT_B   = 1 << 4,
	PIC_FLOPPY_DISK     = 1 << 6,
	PIC_INTERNAL_RTC    = 1 << 8, // Real Time Clock
	PIC_PS2_MOUSE       = 1 << 12,
	PIC_SATA_PRIMARY    = 1 << 14,
    PIC_SATA_SECONDARY  = 1 << 15
};

bool pic_init();
bool pic_destroy();

void pic_enable_interrupt(enum pic_interrupts interrupt);
void pic_disable_interrupt(enum pic_interrupts interrupt);

registers_t *pic_acknowledge_interrupt(registers_t *regs);