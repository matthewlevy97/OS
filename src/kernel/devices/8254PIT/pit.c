#include <devices/8254PIT/pit.h>

bool pit_init()
{
    uint16_t divisor;

    divisor = (uint16_t)(PIT_HERTZ / PIT_FREQUENCY);

    outb(PIT_COMMAND, 0x36);
    outb(PIT_CHANNEL0, divisor & 0xFF);
    outb(PIT_CHANNEL0, (divisor >> 8) & 0xFF);
    return true;
}

bool pit_destroy()
{
    return true;
}