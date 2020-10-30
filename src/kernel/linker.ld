ENTRY(_start)

KERNEL_OFFSET = 0xFFFF800000000000;
KERNEL_START = 0x10000;

SECTIONS
{
	. = KERNEL_START + KERNEL_OFFSET;
	_kernel_start = .;

	.text ALIGN(4K) : AT(ADDR(.text) - KERNEL_OFFSET)
	{
		*(.multiboot)
		*(.text)
	}
	_kernel_text_end = .;

	.data ALIGN(4K) : AT(ADDR(.data) - KERNEL_OFFSET)
	{
		*(.data)

		_driver_initcall_start = .;
		*(.initcall)
		_driver_initcall_end = .;

		_driver_exitcall_start = .;
		*(.exitcall)
		_driver_exitcall_end = .;
	}
	.rodata ALIGN(4K) : AT(ADDR(.rodata) - KERNEL_OFFSET)
	{
		*(.rodata)
		*(.early_boot_structs)
	}
	_kernel_data_end = .;

	.bss ALIGN(4K) : AT(ADDR(.bss) - KERNEL_OFFSET)
	{
		*(COMMON)
		*(.bss)
	}
	_kernel_end = .;
}