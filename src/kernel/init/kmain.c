#include <amd64/asm/paging.h>
#include <init/kmain.h>
#include <klog.h>

void kmain(multiboot_magic_t magic, multiboot_data_t multiboot_data)
{
    klog_init(KERNEL_OFFSET, 80*25*2);

    // Confirm multiboot magic is correct
    if(magic == MULTIBOOT2_MAGIC) {
        klog("Invalid multiboot magic!");
        return;
    }

    klog("Testing\0");

    // TODO: Setup BootStrap Processor's local CPU structure
    // TODO: Re-init GDT, TSS, and IDT
}