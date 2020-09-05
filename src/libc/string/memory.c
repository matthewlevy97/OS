#include <string.h>

void *memcpy(void *dest, const void *src, size_t n)
{
    char *s, *d;

    s = (char*)src;
    d = (char*)dest;
    while(n--) {
        *d++ = *s++;
    }

    return dest;
}