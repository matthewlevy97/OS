#include <amd64/paging.h>
#include <unittests/pmm.h>
#include <mm/pmm.h>
#include <atomic.h>
#include <common.h>
#include <klog.h>

bool pmm_test_page_alloc(void *arg, int *err_code)
{
    struct page *page;
    phys_addr_t addr;

    addr = get_free_page(0);
    if(addr == PMM_INVALID_PAGE) {
        *err_code = 1;
        return false;
    }

    page = physical2page(addr);
    if(NULL == page) {
        *err_code = 2;
        return false;
    }
    klog("VAL: %d\n", atomic_get(&page->ref_count));
    if(atomic_get(&page->ref_count) != 1) {
        *err_code = 3;
        return false;
    }

    free_page(addr);

    return true;
}

bool pmm_test_pfn_convert(void *arg, int *err_code)
{
    struct page* page;
    uintptr_t base_addr;

    base_addr = V2P(ALIGN_PAGE((uintptr_t)&_kernel_start));
    page = physical2page(base_addr);
    if(NULL == page) {
        *err_code = 1;
        return false;
    }

    if(page2physical(page) != base_addr) {
        *err_code = 2;
        return false;
    }
    
    return true;
}