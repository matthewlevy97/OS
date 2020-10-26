#pragma once

#include <unittests/unittest.h>

bool unittest_kmalloc_get_same_memory(void *arg, int *err_code);
bool unittest_kmalloc_combine_memory(void *arg, int *err_code);

static struct unit_test kmalloc_tests[] = {
    {
        .name = "Gets Same Memory [Alloc->Free->Alloc]",
        .func = unittest_kmalloc_get_same_memory,
        .arg  = NULL
    },
    {
        .name = "Combine Free'd Memory Correctly",
        .func = unittest_kmalloc_combine_memory,
        .arg  = NULL
    },
    {NULL}
};