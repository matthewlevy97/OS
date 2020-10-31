#include <string.h>

char *strncpy(char *dest, const char *src, size_t n)
{
    char *d, *s;

    d = dest;
    s = (char*)src;

    while(n-- && *s != '\0') {
        *d++ = *s++;
    }

    while(n--) {
        *d++ = '\0';
    }

    return dest;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
    char *a, *b;
    int diff;

    a = (char*)s1;
    b = (char*)s2;
    while(n-- && *a != '\0' && *b != '\0') {
        diff = *a - *b;
        if(0 != diff) return diff;
    }

    return *a - *b;
}