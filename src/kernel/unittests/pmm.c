#include <amd64/paging.h>
#include <unittests/pmm.h>
#include <mm/pmm.h>
#include <atomic.h>
#include <common.h>
#include <klog.h>

bool unittest_pmm_kernel_base_page_ref_count(void *x, int *err_value)
{
    struct page *p;
    UNUSED(x);
    
    p = pmm_get_page_for_address(V2P(&_kernel_start));
    if(NULL == p || atomic_get(&(p->count)) != 1 ||
        p->physical_page != V2P(&_kernel_start)) {
        *err_value = 1;
        return false;
    }
    return true;
}

bool unittest_pmm_expected_pages(void *x, int *err_value)
{
    uintptr_t tmp;
    uintptr_t pages[5];
    UNUSED(x);

    // Get 5 pages
    for(int i = 0; i < 5; i++) {
        pages[i] = pmm_get_page();
        klog("[PAGE:0x%x]\n", pages[i]);
        if(pages[i] == (uintptr_t)NULL) {
            *err_value = 1;
            goto err_cleanup;
        }
        
        for(int j = 0; j < i; j++) {
            if(pages[j] == pages[i]) {
                *err_value = 2;
                goto err_cleanup;
            }
        }
    }

    // Release 2 pages
    pmm_release_page(pages[0]);
    pmm_release_page(pages[1]);

    // Reacquire (Expect same pages)
    tmp = pmm_get_page();
    klog("[PAGE2:0x%x]\n", tmp);
    if(tmp != pages[1]) {
        pmm_release_page(tmp);
        pages[0] = NULL;
        *err_value = 3;
        goto err_cleanup;
    }

    tmp = pmm_get_page();
    klog("[PAGE3:0x%x]\n", tmp);
    if(tmp != pages[0]) {
        pmm_release_page(tmp);
        *err_value = 4;
        goto err_cleanup;
    }

    for(int i = 0; i < 5; i++) 
        if(pages[i]) pmm_release_page(pages[i]);
    return true;
err_cleanup:
    for(int i = 0; i < 5; i++)
        if(pages[i]) pmm_release_page(pages[i]);
    return false;
}