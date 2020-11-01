#include <amd64/init.h>
#include <drivers/drivers.h>
#include <init/kmain.h>
#include <mm/kmalloc.h>
#include <mm/paging.h>
#include <mm/pmm.h>
#include <mm/vm_layout.h>
#include <process/scheduler.h>
#include <interrupt.h>
#include <klog.h>
#include <string.h>

static void init_mm(multiboot_header_t multiboot_data);
static void stage2();

#ifdef RUN_UNITTESTS
#include <unittests/unittest.h>

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

__no_return void kmain(multiboot_magic_t magic, multiboot_header_t multiboot_data)
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
     * Initialize process scheduler and start interrupts
     */
    struct process *stage2_proc = process_create("idle", &stage2, 0);
    scheduler_init(stage2_proc);

    __unreachable;
}

__no_return static void stage2()
{
    klog("Kernel Stage 2 Loaded!\n");

    /**
     * Initialize kernel drivers
     */
    init_drivers();
    
#ifdef RUN_UNITTESTS
    unittest_kernel();
#endif

    scheduler_add(process_create("init", NULL, PROCESS_CREATE_USER));

    // Kernel idle task
    scheduler_set_priority(process_this(), PROCESS_PRIORITY_IDLE);
    while(1);
}

static __inline void init_mm(multiboot_header_t multiboot_data)
{
    struct vm_map *vm_map;

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

    vm_map = paging_init();
    if(NULL == vm_map) {
        KPANIC("Failed to initialize paging!\n");
        return;
    }
    klog("Initialized Paging!\n");

    if(!kmalloc_init(vm_map)) {
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