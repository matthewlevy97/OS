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

void kmain(multiboot_magic_t magic, multiboot_header_t multiboot_data)
{
    klog_init((void*)P2V(0xB8000), 80*25*2);
    
    // Remap multiboot data to 2nd physical page in memory
    memcpy(P2V(PAGE_SIZE), multiboot_data, multiboot_data->size);
    multiboot_data = P2V(PAGE_SIZE);

    // TODO: Remap multiboot loaded modules to lower memory

    // Confirm multiboot magic is correct
    if(magic != MULTIBOOT2_MAGIC) {
        klog("Invalid multiboot magic!");
        return;
    }
    
    if(!pmm_init(multiboot_data)) {
        klog("Failed to initialize PMM!");
        return;
    }
#ifdef RUN_UNITTESTS
    size_t num_tests, failed;
    if(!run_unittests(UNIT_TEST_PMM, &num_tests, &failed)) {
        klog("[PMM Unit-Tests] Failed %d/%d\n", failed, num_tests);
        return;
    }
    klog("[PMM Unit-Tests] Passed All %d Tests\n", num_tests);
#endif

    if(!paging_init()) {
        klog("Paging failed to initialize!");
        return;
    }

    if(NULL == amd64_init_core(NULL)) {
        klog("Bootstrap CPU Core failed to initialize!");
        return;
    }

    klog("Bootstrap CPU Core Initialized!");
    
    interrupt_enable();
    while(1);
}