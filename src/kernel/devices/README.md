# Hardware Devices
All code for handling physical hardware devices goes in this folder

## Adding Hardware Device To Kernel
None of the devices in this folder are included/compiled with the kernel. To add
a device driver, it must be included by a Makefile during compilation. This can 
be done by following the instructions in *devices/make.config*

## Architecture Specific Code
Use the *ARCH_XXX* defines for handling of architecture specific code. If possible,
include all header files for architecture specific code in the *devices/devices.h*
file inside of the correct *ARCH_XXX* define. This decreases the number of places
that need to be changed should a device be used for multiple system architectures
(ex. same Programmable Interrupt Controller hardware used for both 32-bit and 
64-bit Intel CPUs)