#include <amd64/asm/paging.h>
#include <amd64/interrupt.h>
#include <amd64/init.h>
#include <init/kmain.h>
#include <klog.h>

void kmain(multiboot_magic_t magic, multiboot_data_t multiboot_data)
{
    klog_init((void*)KERNEL_OFFSET, 80*25*2);

    // Confirm multiboot magic is correct
    if(magic != MULTIBOOT2_MAGIC) {
        klog("Invalid multiboot magic!");
        return;
    }
    
    if(NULL == amd64_init_core(NULL)) {
        klog("Bootstrap CPU Core failed to initialize!");
        return;
    }

    klog("Bootstrap CPU Core Initialized!");

    interrupt_enable();
}