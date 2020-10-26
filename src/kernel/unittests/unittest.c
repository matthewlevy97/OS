#include <unittests/unittest.h>
#include <unittests/pmm.h>
#include <unittests/kmalloc.h>
#include <unittests/pci.h>
#include <klog.h>

static struct unit_test *tests[UNIT_TEST_NUM] = {
    [UNIT_TEST_PMM]     = pmm_tests,
    [UNIT_TEST_KMALLOC] = kmalloc_tests,
    [UNIT_TEST_PCI]     = pci_tests
};

static void run_test(struct unit_test*, size_t*, size_t*);

bool run_unittests(enum unit_test_type type, size_t *num_tests, size_t *failed)
{
    *num_tests = 0;
    *failed    = 0;

    run_test(tests[type], num_tests, failed);

    return (0 == *failed);
}

static void run_test(struct unit_test *tests, size_t *num_tests, size_t *failed)
{
    struct unit_test *test;
    int err_value;

    test = tests;
    while(test != NULL && test->func != NULL) {
        *num_tests += 1;
        err_value   = 0;
        if(!test->func(test->arg, &err_value)) {
            klog("[UnitTest] Failed[%d]: %s\n", err_value, test->name);
            *failed += 1;
        }
        test++;
    }
}