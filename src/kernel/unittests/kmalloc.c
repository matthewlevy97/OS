#include <unittests/unittest.h>
#include <mm/kmalloc.h>

bool unittest_kmalloc_get_same_memory(void *arg, int *err_code)
{
    char *a, *b;

    a = kmalloc(8, GFP_ZERO);
    kfree(a);
    b = kmalloc(8, GFP_ZERO);
    kfree(b);

    if(a != b) {
        *err_code = 1;
    }

    return !(*err_code);
}

bool unittest_kmalloc_combine_memory(void *arg, int *err_code)
{
    char *a, *b, *c;

    a = kmalloc(8, GFP_ZERO);
    b = kmalloc(8, GFP_ZERO);
    c = kmalloc(8, GFP_ZERO);

    kfree(a);
    kfree(b);
    
    a = kmalloc(16, GFP_ZERO);
    kfree(a);
    kfree(c);

    if(a >= c) {
        *err_code = 1;
    }

    return !(*err_code);
}