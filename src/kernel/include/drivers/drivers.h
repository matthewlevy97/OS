#pragma once

#include <common.h>
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
 * For Driver Initialization
 */
typedef bool (*initcall_t)(void);

#define __define_initcall(fn)                                           \
	static initcall_t __used __attribute__((__section__(".initcall")))  \
    __initcall_##fn = fn;
#define __define_exitcall(fn)                                           \
	static initcall_t __used __attribute__((__section__(".exitcall")))  \
    __initcall_##fn = fn;

/**
 * NOTE: Do not use these macros in header files, only source files
 */
#define MODULE_INIT(fn) __define_initcall(fn)
#define MODULE_EXIT(fn) __define_exitcall(fn)

void init_drivers();
void destroy_drivers();