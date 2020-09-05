#include <klog.h>
#include <ring_buffer.h>

static ring_buffer_t ring_buffer;

int klog_init(void *buffer, size_t length)
{
    if(!ring_buffer_init(&ring_buffer, buffer, length, 0))
        return -1;

    return 0;
}

void klog(const char *msg)
{
    char *ptr;

    ptr = (char*)msg;
    while(*ptr != '\0') {
        ring_buffer_push(&ring_buffer, *ptr++);
        ring_buffer_push(&ring_buffer, 0x07);
    }
}