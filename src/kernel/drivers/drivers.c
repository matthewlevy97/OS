#include <drivers/drivers.h>
#include <klog.h>

extern uintptr_t _driver_initcall_start, _driver_initcall_end;
extern uintptr_t _driver_exitcall_start, _driver_exitcall_end;

void init_drivers()
{
    initcall_t *func;
    bool ret;

    func = (initcall_t*)&_driver_initcall_start;
    while((uintptr_t)func != (uintptr_t)&_driver_initcall_end) {
        ret = (*func)();
        if(false == ret) {
            KPANIC("Initialization function 0x%x failed!\n", *func);
        }

        func++;
    }
}

void destroy_drivers()
{
    initcall_t *func;

    func = (initcall_t*)&_driver_exitcall_start;
    while((uintptr_t)func != (uintptr_t)&_driver_exitcall_end) {
        (*func)();
        func++;
    }
}