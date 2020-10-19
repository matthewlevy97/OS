#include <debug.h>

__inline void debug_dump_cpu_state()
{
#ifdef ARCH_x86_64
    // TODO: Dump CPU state (register, backtrace, etc.)
#else
    klog("Architecture not supported for dumping CPU state!\n");
#endif
}