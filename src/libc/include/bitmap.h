#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define BITMAP_BASE_UNIT               uint32_t
#define BITMAP_BITS_IN_UNIT            (8 * sizeof(BITMAP_BASE_UNIT))
#define BITMAP_LENGTH(bits)            ((bits) / BITMAP_BITS_IN_UNIT + \
    (((bits) % BITMAP_BITS_IN_UNIT) ? 1 : 0))

#define __BITMAP_GET_INDEX(n)          ((n) / (BITMAP_BITS_IN_UNIT))

#define __BITMAP_GET_BIT(n, b)         ((n) >> (b) & 0x1)
#define __BITMAP_SET_BIT(n, b)         ((n) | (1 << (b)))
#define __BITMAP_CLEAR_BIT(n, b)       ((n) & ~(1 << (b)))
#define __BITMAP_TOGGLE_BIT(n, b)      ((n) ^ (1 << (b)))

typedef struct __bitmap {
    BITMAP_BASE_UNIT *ptr;
    size_t len_bits;
} bitmap_t;

bool bitmap_init(bitmap_t*, void *ptr, size_t num_bits);

bool bitmap_set(bitmap_t*, size_t);
bool bitmap_clear(bitmap_t*, size_t);
bool bitmap_toggle(bitmap_t*, size_t, char*);

bool bitmap_get(bitmap_t*, size_t, char*);