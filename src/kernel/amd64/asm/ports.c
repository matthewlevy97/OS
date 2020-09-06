#include <amd64/asm/ports.h>
#include <common.h>

__inline void outb(uint16_t port, uint8_t value)
{
    __asm__ __volatile__ ("outb %b0, %w1": :"a" (value), "Nd" (port));
}

__inline void outw(uint16_t port, uint16_t value)
{
    __asm__ __volatile__ ("outw %w0, %w1": :"a" (value), "Nd" (port));
}

__inline void outl(uint16_t port, uint32_t value)
{
    __asm__ __volatile__ ("outl %0, %w1": :"a" (value), "Nd" (port));
}

__inline uint8_t inb(uint16_t port)
{
    uint8_t v;
    __asm__ __volatile__ ("inb %w1, %0":"=a" (v):"Nd" (port));
    return v;
}

__inline uint16_t inw(uint16_t port)
{
    uint16_t v;
    __asm__ __volatile__ ("inw %w1, %0":"=a" (v):"Nd" (port));
    return v;
}

__inline uint32_t inl(uint16_t port)
{
    uint32_t v;
    __asm__ __volatile__ ("inl %w1, %0":"=a" (v):"Nd" (port));
    return v;
}