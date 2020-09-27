#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <boot/multiboot2.h>

typedef enum {
    MULTIBOOT_TYPE_CMD_LINE   = 1,
    MULTIBOOT_TYPE_MODULES    = 2,
    MULTIBOOT_TYPE_MEMORY_MAP = 6,
} multiboot_tag_type_t;

struct multiboot_header_tag {
    uint32_t size;
    uint32_t reserved;
};

struct multiboot_tag {
    uint32_t type;
    uint32_t size;
};

enum multiboot_mmap_entry_types {
    MULTIBOOT_MMAP_TYPE_IGNORE    = 0,
    MULTIBOOT_MMAP_TYPE_AVAILABLE = 1,
    MULTIBOOT_MMAP_TYPE_RESERVE   = 2,
    MULTIBOOT_MMAP_TYPE_ACPI      = 3,
    MULTIBOOT_MMAP_TYPE_NVS       = 4,
    MULTIBOOT_MMAP_TYPE_DEFECTIVE = 5
};
struct multiboot_mmap_entry_tag {
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
    uint32_t reserved;
};
struct multiboot_mmap_tag {
    struct multiboot_tag header;
    uint32_t entry_size;
    uint32_t entry_version;
    struct multiboot_mmap_entry_tag entries[];
};

typedef uint32_t multiboot_magic_t;
typedef struct multiboot_header_tag * multiboot_header_t;

struct multiboot_tag *multiboot_get_tag(multiboot_header_t, multiboot_tag_type_t);