#include <amd64/interrupt.h>
#include <amd64/init.h>
#include <init/kmain.h>
#include <mm/paging.h>
#include <mm/pmm.h>
#include <mm/vm_layout.h>
#include <klog.h>
#include <string.h>

#ifdef RUN_UNITTESTS
#include <unittests/unittest.h>
#endif

static struct vm_map init_vmmap;

void kmain(multiboot_magic_t magic, multiboot_header_t multiboot_data)
{
    interrupt_disable();

    klog_init((void*)P2V(0xB8000), 80*25*2);

    // Remap multiboot data to directly after kernel
    memcpy((void*)P2V(ALIGN_LOG2((uintptr_t)&_kernel_end, PAGE_SIZE)),
        multiboot_data, multiboot_data->size);
    multiboot_data = (multiboot_header_t)P2V(ALIGN_LOG2(
        (uintptr_t)&_kernel_end, PAGE_SIZE));
    
    // TODO: Remap multiboot loaded modules to right after multiboot
    
    // Confirm multiboot magic is correct
    if(magic != MULTIBOOT2_MAGIC) {
        klog("Invalid multiboot magic!");
        return;
    }

    if(NULL == amd64_init_core(NULL)) {
        klog("Bootstrap CPU Core failed to initialize!\n");
        return;
    }
    klog("Bootstrap CPU Core Initialized!\n");

    if(!pmm_init(multiboot_data)) {
        klog("Failed to initialize PMM!\n");
        return;
    }
    klog("Initialized PMM!\n");
#ifdef RUN_UNITTESTS
    size_t num_tests, failed;
    if(!run_unittests(UNIT_TEST_PMM, &num_tests, &failed)) {
        klog("[PMM Unit-Tests] Failed %d/%d\n", failed, num_tests);
        return;
    }
    klog("[PMM Unit-Tests] Passed All %d Tests\n", num_tests);
#endif
    
    if(!paging_init(&init_vmmap)) {
        klog("Paging failed to initialize!\n");
        return;
    }
    klog("Initialized Paging!\n");

    interrupt_enable();

    klog("Kernel Loaded!\n");
    while(1);
}