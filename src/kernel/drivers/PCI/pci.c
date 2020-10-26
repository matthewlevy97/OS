#include <drivers/PCI/pci.h>
#include <mm/kmalloc.h>

static struct pci_driver *drivers;

struct pci_dev *pci_dev_create(uint8_t bus, uint8_t device);
void pci_dev_destroy(struct pci_dev *dev);

static uint32_t read_config32(uint8_t bus, uint8_t slot, uint8_t func,
    uint8_t offset);

static uint16_t pci_get_device_id(uint8_t bus, uint8_t device);
static uint16_t pci_get_vendor_id(uint8_t bus, uint8_t device);
static uint32_t pci_get_class_id(uint8_t bus, uint8_t device);

bool pci_init()
{
    drivers = NULL;

    return true;
}

bool pci_destroy()
{
    // TODO: Free drivers

    return true;
}

void pci_scan()
{
    struct pci_dev *dev;
    for(int bus = 0; bus < 256; bus++) {
        for(int device = 0; device < 32; device++) {
            if(pci_get_vendor_id(bus, device) != 0xFFFF) {
                /*
                klog("Found Device! [Bus: %d, Device: %d, Class: %d]\n", bus,
                    device, pci_get_class_id(bus, device));
                */

                dev = pci_dev_create(bus, device);
                
                // TODO: Improve this really inefficient code
                list_foreach(drivers, driver) {
                    for(size_t i = 0; driver->id_table[i].device; i++) {
                        if(driver->id_table[i].class_id ==
                            (dev->class_id & driver->id_table[i].class_mask)) {
                            
                            if(driver->probe)
                                driver->probe(dev, &driver->id_table[i]);
                        }
                    }
                }

                pci_dev_destroy(dev);
            }
        }
    }
}

int pci_register_driver(struct pci_driver *driver)
{
    if(NULL == driver)
        return -1;
    
    if(!list_append(&drivers, driver))
        return -2;

    return 0;
}

void pci_unregister_driver(struct pci_driver *driver)
{
    if(NULL == driver)
        return;
    
    list_remove(&driver);
}

struct pci_dev *pci_dev_create(uint8_t bus, uint8_t device)
{
    struct pci_dev *dev;

    dev = kmalloc(sizeof(*dev), GFP_KERNEL | GFP_ZERO);
    if(NULL == dev)
        return NULL;
    
    dev->device_id = pci_get_device_id(bus, device);
    dev->vendor_id = pci_get_vendor_id(bus, device);
    dev->class_id  = pci_get_class_id(bus, device);

    return dev;
}

void pci_dev_destroy(struct pci_dev *dev)
{
    kfree(dev);
}

static uint32_t read_config32(uint8_t bus, uint8_t slot, uint8_t func,
    uint8_t offset)
{
    uint32_t address;

    address = (((uint32_t)bus) << 16) | (((uint32_t)slot) << 11) |
        (((uint32_t)func) << 8) | (((uint32_t)offset) & 0xFC) |
        ((uint32_t)PCI_ADDRESS_ENABLE_BIT);
    
    outl(PCI_CONFIG_ADDRESS, address);
    // NOTE: (offset & 2) * 8) = 0 will choose the first word of the 32 bits register
    return inl(PCI_CONFIG_DATA);
}

static __inline uint16_t pci_get_device_id(uint8_t bus, uint8_t device)
{
	return read_config32(bus, device, 0,
        offsetof(struct pci_config_header, device_id)) >> 16;
}

static __inline uint16_t pci_get_vendor_id(uint8_t bus, uint8_t device)
{
	return read_config32(bus, device, 0,
        offsetof(struct pci_config_header, vendor_id)) & 0xFFFF;
}

static __inline uint32_t pci_get_class_id(uint8_t bus, uint8_t device)
{
    return read_config32(bus, device, 0,
        offsetof(struct pci_config_header, device_class));
}