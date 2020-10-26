# Kernel Drivers
Unlike the *devices/* folder which holds interfaces for hardware devices, the 
drivers folder holds kernel drivers to support more abstract concepts (ex. PCIe,
ACPI, etc.)

## Compiling
Unlike the *devices/* folder where each device must be explicitly included in
the kernel, the drivers here are all included by default with the kernel. This
means that if a new driver is added, it must be included in the 
*drivers/make.config* file.