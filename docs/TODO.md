# TODO
- Map all physical memory into virtual memory
	- Currently only does first 1G of physical memory w/ 2MB pages
- Decide if we want page level security in kernel (ie NX/R-ONLY)
	- If not, just do large pages (2MB), otherwise need to convert
		back to 4kb pages after kernel loads
- Setup Timer interrupt
- Finish paging/virtual memory code
	- Abstract arch specific paging code out
	- General paging code for AVL trees, specific code for with "actual paging"
- Setup CMOS/RTC
- Setup serial output
- Start adding unit tests again to code
- kmem_cache -> SLAB allocator