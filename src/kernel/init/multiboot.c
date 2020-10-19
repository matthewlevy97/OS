#include <init/multiboot.h>
#include <klog.h>

struct multiboot_tag *multiboot_get_tag(multiboot_header_t data,
    multiboot_tag_type_t type)
{
    struct multiboot_tag *tag;
    char *ptr;

    ptr = (char*)data;
    ptr += sizeof(struct multiboot_header_tag);
    while((uintptr_t)ptr < (uintptr_t)(data + data->size)) {
        tag = (struct multiboot_tag *)ptr;
        if(tag->type == type)
            return tag;
        ptr += tag->size + ((tag->size & 7) ? 8 - (tag->size & 7) : 0);
    }

    return NULL;
}