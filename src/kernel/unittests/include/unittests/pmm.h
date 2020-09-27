#pragma once

#include <unittests/unittest.h>

bool unittest_pmm_kernel_base_page_ref_count(void*, int*);
bool unittest_pmm_expected_pages(void*, int*);

static struct unit_test pmm_tests[] = {
    {
        .name = "Kernel Base Struct Page Reference Count == 1",
        .func = unittest_pmm_kernel_base_page_ref_count,
        .arg  = NULL
    },
    {
        .name = "pmm_get_page() stack push/pop correctly",
        .func = unittest_pmm_expected_pages,
        .arg  = NULL
    },
    {NULL}
};