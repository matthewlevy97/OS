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

void *memset(void *s, int c, size_t n)
{
    char *ptr;

    ptr = (char*)s;
    while(n--) {
        *ptr++ = (char)c;
    }

    return s;
}