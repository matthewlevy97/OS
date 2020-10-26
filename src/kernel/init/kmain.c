#include <amd64/init.h>
#include <init/kmain.h>
#include <mm/kmalloc.h>
#include <mm/paging.h>
#include <mm/pmm.h>
#include <mm/vm_layout.h>
#include <interrupt.h>
#include <klog.h>
#include <string.h>

#include <drivers/PCI/pci.h>

#ifdef RUN_UNITTESTS
#include <unittests/unittest.h>
#endif

static struct vm_map init_vmmap;

#ifdef RUN_UNITTESTS
/**
 * Run Unit Tests on all components
 */
static __inline void unittest_kernel()
{
    // PCI Unit Tests
    do {
        size_t num_tests, failed;
        if(!run_unittests(UNIT_TEST_PCI, &num_tests, &failed)) {
            KPANIC("[PCI Unit-Tests] Failed %d/%d\n", failed, num_tests);
            return;
        }
        klog("[PCI Unit-Tests] Passed All %d Tests\n", num_tests);
    } while(0);
}
#endif

static __inline void init_mm(multiboot_header_t multiboot_data)
{
    if(!pmm_init(multiboot_data)) {
        KPANIC("Failed to initialize PMM!\n");
        return;
    }
    klog("Initialized PMM!\n");
#ifdef RUN_UNITTESTS
    do {
        size_t num_tests, failed;
        if(!run_unittests(UNIT_TEST_PMM, &num_tests, &failed)) {
            KPANIC("[PMM Unit-Tests] Failed %d/%d\n", failed, num_tests);
            return;
        }
        klog("[PMM Unit-Tests] Passed All %d Tests\n", num_tests);
    } while(0);
#endif

    if(!paging_init(&init_vmmap)) {
        KPANIC("Failed to initialize paging!\n");
        return;
    }
    klog("Initialized Paging!\n");

    if(!kmalloc_init(&init_vmmap)) {
        KPANIC("Failed to initialize KMalloc!");
    }
    klog("Initialized KMalloc!\n");
#ifdef RUN_UNITTESTS
    do {
        size_t num_tests, failed;
        if(!run_unittests(UNIT_TEST_KMALLOC, &num_tests, &failed)) {
            KPANIC("[KMalloc Unit-Tests] Failed %d/%d\n", failed, num_tests);
            return;
        }
        klog("[KMalloc Unit-Tests] Passed All %d Tests\n", num_tests);
    } while(0);
#endif
}

void kmain(multiboot_magic_t magic, multiboot_header_t multiboot_data)
{
    interrupt_disable();

    /**
     * Initialize KLog Ring Buffer - 0xB8000 maps to VGA output
     */
    klog_init((void*)P2V(0xB8000), 80*25*2);

    /**
     * Remap multiboot data to directly after kernel
     */
    memcpy((void*)P2V(ALIGN_LOG2((uintptr_t)&_kernel_end, PAGE_SIZE)),
        multiboot_data, multiboot_data->size);
    multiboot_data = (multiboot_header_t)P2V(ALIGN_LOG2(
        (uintptr_t)&_kernel_end, PAGE_SIZE));
    
    // TODO: Remap multiboot loaded modules to right after multiboot
    
    /**
     * Confirm multiboot magic is correct
     */
    if(magic != MULTIBOOT2_MAGIC) {
        KPANIC("Invalid multiboot magic!");
        return;
    }

    /**
     * Initialize CPU
     */
    if(NULL == amd64_init_core(NULL)) {
        KPANIC("Bootstrap CPU Core failed to initialize!\n");
        return;
    }
    klog("Bootstrap CPU Core Initialized!\n");

    /**
     * Initialize Memory Functions
     */
    init_mm(multiboot_data);
    
    /**
     * Kernel is now live!
     */
    interrupt_enable();
    klog("Kernel Loaded!\n");

#ifdef RUN_UNITTESTS
    unittest_kernel();
#endif
    
    while(1);
}