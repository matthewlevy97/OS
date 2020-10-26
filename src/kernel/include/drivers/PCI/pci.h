#pragma once

#include <drivers/drivers.h>

#define PCI_CONFIG_ADDRESS             (0xCF8)
#define PCI_CONFIG_DATA                (0xCFC)
#define PCI_ADDRESS_ENABLE_BIT         (0x80000000)

#define PCI_MASK_SUBCLASS              (~((1 << 16) - 1))
#define PCI_MASK_CLASS                 (~((1 << 24) - 1))

#define PCI_ANY_ID                     (0xFFFFFFFF)

#define PCI_DEVICE(vend, dev)                               \
	.vendor = (vend), .device = (dev),                      \
	.subvendor = PCI_ANY_ID, .subdevice = PCI_ANY_ID
#define PCI_DEVICE_CLASS(dev_class, dev_class_mask)         \
	.class_id = (dev_class), .class_mask = (dev_class_mask),\
	.vendor = PCI_ANY_ID, .device = PCI_ANY_ID,             \
	.subvendor = PCI_ANY_ID, .subdevice = PCI_ANY_ID

struct pci_device_id {
    uint32_t  vendor, device;
    uint32_t  subvendor, subdevice;
    union {
        uint32_t class_id;
        struct {
            uint8_t  revision_id;
            uint8_t  program_if;
            uint8_t  subclass;
            uint8_t  class_code;
        };
    };
    uint32_t  class_mask;
};

struct pci_dev {
    uint16_t device_id, vendor_id;
    union {
        uint32_t class_id;
        struct {
            uint8_t  revision_id;
            uint8_t  program_if;
            uint8_t  subclass;
            uint8_t  class_code;
        };
    };
};

struct pci_driver {
    ListEntry_HEAD

    const char *name;
    const struct pci_device_id *id_table;
    int (*probe)(struct pci_dev *dev, const struct pci_device_id *id);
};

/**
 * MF:
 *  If MF = 1 Then this device has multiple functions.
 * Header Type:
 *  0=Standard Header, 1=PCI-to-PCI Bridge, 2=CardBus Bridge
 */
typedef struct {
    uint8_t type : 7;
    uint8_t mf   : 1;
} pci_header_type_register_t;

/**
 * BIST Capable:
 *  Will return 1 the device supports BIST.
 * Start BIST:
 *  When set to 1 the BIST is invoked. This bit is reset when BIST completes.
 *  If BIST does not complete after 2 seconds the device should be failed by
 *  system software.
 * Completion Code:
 *  Will return 0, after BIST execution, if the test completed successfully.
 */
typedef struct {
    uint8_t completion_code : 4;
    uint8_t _reserved       : 2;
    uint8_t start_bist      : 1;
    uint8_t bist_capable    : 1;
} pci_bist_register_t;

struct pci_config_header {
    uint16_t vendor_id;
    uint16_t device_id;

    uint16_t command;
    uint16_t status;

    union {
        uint32_t device_class;
        struct {
            uint8_t  revision_id;
            uint8_t  program_if;
            uint8_t  subclass;
            uint8_t  class_code;
        };
    };

    uint8_t  cache_line_size;
    uint8_t  latency_timer;
    uint8_t  header_type;
    uint8_t  BIST;

    char     header_extra[];
};

struct pci_header_type_0 {
    uint32_t bar_0;
    uint32_t bar_1;
    uint32_t bar_2;
    uint32_t bar_3;
    uint32_t bar_4;
    uint32_t bar_5;
    uint32_t cardbus_cis_pointer;
    
    uint16_t subsystem_vendor_id;
    uint16_t subsystem_id;
    
    uint32_t extension_rom_bar;
    
    uint8_t  capabilities_ptr;
    uint32_t _reserved : 24;
    uint32_t __reserved;
    
    uint8_t  interrupt_line;
    uint8_t  interrupt_pin;
    uint8_t  min_grant;
    uint8_t  max_latency;
};

struct pci_header_type_1 {
    uint32_t bar_0;
    uint32_t bar_1;
    
    uint8_t  primary_bus_number;
    uint8_t  secondary_bus_number;
    uint8_t  subordinate_bus_number;
    uint8_t  secondary_latency_timer;
    
    uint8_t  io_base;
    uint8_t  io_limit;
    uint16_t secondary_status;
    
    uint16_t memory_base;
    uint16_t memory_limit;
    
    uint16_t prefetchable_memory_base;
    uint16_t prefetchable_memory_limit;
    
    uint32_t prefetchable_base_upper;
    uint32_t prefetchable_limit_upper;
    
    uint16_t io_base_upper;
    uint16_t io_base_lower;
    
    uint8_t  capabilities_ptr;
    uint32_t _reserved : 24;
    
    uint32_t expansion_rom_bar;

    uint8_t  interrupt_line;
    uint8_t  interrupt_pin;
    uint16_t bridge_control;
};

bool pci_init();
bool pci_destroy();

void pci_scan();

int pci_register_driver(struct pci_driver *driver);
void pci_unregister_driver(struct pci_driver *driver);