#pragma once

#include <stdbool.h>
#include <stddef.h>

enum unit_test_type {
    UNIT_TEST_PMM,
    UNIT_TEST_NUM
};

struct unit_test {
    char *name;
    bool (*func)(void*,int*);
    void *arg;
};

bool run_unittests(enum unit_test_type type, size_t *num_tests, size_t *failed);