# Design Goals
- Small kernel, many drivers
    - Drivers for all specific concepts (PCI, ACPI, Timer, etc.)
    - Drivers loaded from RAM Disk
    - Kernel function wrappers for non-specific (Interrupts, Memory Management, Multitasking, etc.)
- Runs amd64 (and 32-bit cross compatability possibly)
    - Note: This means certain memory management (ie paging) might need to be in arch specific folders
    - Supports: syscall/sysret
- Multiboot will be considered deprecated soon
    - Support Multiboot, but make easily able to transition to UEFI when needed