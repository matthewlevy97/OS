#pragma once

#include <stdbool.h>
#include <stdint.h>

/**
 * All architecture specific import need to be here
 * The correct imports will be chosen via DEFINE directives
 */
#ifdef ARCH_x86_64
#include <amd64/cpu.h>
#endif

/**
 * These functions need to be defined for all ports of the kernel to
 *  other architectures
 */
extern void outb(uint16_t port, uint8_t value);
extern void outw(uint16_t port, uint16_t value);
extern void outl(uint16_t port, uint32_t value);

extern uint8_t inb(uint16_t port);
extern uint16_t inw(uint16_t port);
extern uint32_t inl(uint16_t port);

/**
 * Common interface functions that MUST be defined in all drivers
 *  - bool [DRIVER]_init();
 *  - bool [DRIVER]_destroy();
 */