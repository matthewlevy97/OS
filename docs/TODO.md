# TODO
- Paging code (Virtual MM)

- Map all physical memory into virtual memory

- Decide if we want page level security in kernel (ie NX/R-ONLY)
	- If not, just do large pages (2MB), otherwise need to convert
		back to 4kb pages after kernel loads

VMM - AVL tree of virtual memory

- Setup Timer interrupt
- Finish paging/virtual memory code
- Update vm_layout
- Setup CMOS/RTC
- Setup serial output
- Start adding unit tests again to code
- kmem_cache -> SLAB allocator