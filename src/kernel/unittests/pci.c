#include <unittests/pci.h>
#include <drivers/PCI/pci.h>

int pci_test_probe(struct pci_dev *dev, const struct pci_device_id *id)
{
    klog("Probe For PCI Test Hit!");
    
    return 0;
}

bool unittest_pci_detect(void *arg, int *err_code)
{
    // 0x03000000 == VGA Controller
    struct pci_device_id pci_ids[] = {
        {PCI_DEVICE_CLASS(0x03000000, PCI_MASK_CLASS)},
        {NULL}
    };
    struct pci_driver driver = {
        .name = "Test Driver",
        .id_table = pci_ids,
        .probe = pci_test_probe
    };

    if(pci_register_driver(&driver))
        return false;
    pci_scan();
    pci_unregister_driver(&driver);
    pci_scan();

    return true;
}