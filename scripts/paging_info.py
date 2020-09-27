class Paging():
    NX = (1 << 63)
    def map_page(self, virtual_address, physical_address, flags):
        page_table_offset = (virtual_address >> 12) & 0x1FF
        page_dir_offset   = (virtual_address >> 21) & 0x1FF
        pd_pointer_offset = (virtual_address >> 30) & 0x1FF
        pml4_offset       = (virtual_address >> 39) & 0x1FF

        print("Virtual Address: 0x{:x}".format(virtual_address))
        print("page_dir[{}][{}][{}][{}] = 0x{:x}".format(pml4_offset,
            pd_pointer_offset, page_dir_offset, page_table_offset,
            (physical_address & (~0xFFF)) | flags))

paging = Paging()
paging.map_page(0xFFFF800000000000, 0xcafebabe, 0)