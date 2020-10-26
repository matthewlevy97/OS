#pragma once

#include <unittests/unittest.h>

bool unittest_pci_detect(void *arg, int *err_code);

static struct unit_test pci_tests[] = {
    {
        .name = "Detect PCI Device",
        .func = unittest_pci_detect,
        .arg  = NULL
    },
    {NULL}
};