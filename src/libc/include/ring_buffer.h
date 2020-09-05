#pragma once

#include <sys/types.h>

enum {
    RING_BUFFER_OVERWRITE        = (1 << 0),
};

enum {
    RING_BUFFER_ESUCCESS,
    RING_BUFFER_EINVALID,
    RING_BUFFER_EFULL,
    RING_BUFFER_EEMPTY
};

typedef struct {
    size_t wr_head, rd_head;
    size_t length;
    size_t flags;
    void *buffer;
} ring_buffer_t;

ring_buffer_t *ring_buffer_init(ring_buffer_t*, void*, size_t, int);

int ring_buffer_push(ring_buffer_t *rb, char c);
int ring_buffer_pop(ring_buffer_t *rb, char *c);