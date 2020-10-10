#include <bitmap.h>

bool bitmap_init(bitmap_t *bitmap, void *ptr, size_t num_bits)
{
    if(NULL == bitmap)
        return false;

    bitmap->ptr      = ptr;
    bitmap->len_bits = num_bits;

    return true;
}

bool bitmap_set(bitmap_t *bitmap, size_t i)
{
    size_t pos, bit;

    if(NULL == bitmap || i >= bitmap->len_bits) return false;

    pos = i / BITMAP_BITS_IN_UNIT;
    bit = i % BITMAP_BITS_IN_UNIT;

    bitmap->ptr[pos] = __BITMAP_SET_BIT(bitmap->ptr[pos], bit);
    return true;
}

bool bitmap_clear(bitmap_t *bitmap, size_t i)
{
    size_t pos, bit;

    if(NULL == bitmap || i >= bitmap->len_bits) return false;

    pos = i / BITMAP_BITS_IN_UNIT;
    bit = i % BITMAP_BITS_IN_UNIT;

    bitmap->ptr[pos] = __BITMAP_CLEAR_BIT(bitmap->ptr[pos], bit);
    return true;
}

bool bitmap_toggle(bitmap_t *bitmap, size_t i, char *b)
{
    size_t pos, bit;

    if(NULL == bitmap || i >= bitmap->len_bits) return false;

    pos = i / BITMAP_BITS_IN_UNIT;
    bit = i % BITMAP_BITS_IN_UNIT;

    bitmap->ptr[pos] = __BITMAP_TOGGLE_BIT(bitmap->ptr[pos], bit);
    if(b)
        *b = (char)__BITMAP_GET_BIT(bitmap->ptr[pos], bit);
    
    return true;
}

bool bitmap_get(bitmap_t *bitmap, size_t i, char *b)
{
    size_t pos, bit;

    if(NULL == bitmap || i >= bitmap->len_bits) return false;

    pos = i / BITMAP_BITS_IN_UNIT;
    bit = i % BITMAP_BITS_IN_UNIT;

    if(b)
        *b = (char)__BITMAP_GET_BIT(bitmap->ptr[pos], bit);
    
    return true;
}