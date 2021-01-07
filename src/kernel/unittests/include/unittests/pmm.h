#pragma once

#include <unittests/unittest.h>

bool pmm_test_page_alloc(void *arg, int *err_code);
bool pmm_test_pfn_convert(void *arg, int *err_code);

static struct unit_test pmm_tests[] = {
    {
        .name = "Convert Physical Address->Struct Page*->Physical Address",
        .func = pmm_test_pfn_convert,
        .arg  = NULL
    },
    {
        .name = "Allocate physical page",
        .func = pmm_test_page_alloc,
        .arg  = NULL
    },
    {NULL}
};