#include <ring_buffer.h>
#include <stdbool.h>
#include <string.h>

static size_t mask(ring_buffer_t *rb, size_t index);
static bool   empty(ring_buffer_t *rb);
static bool   full(ring_buffer_t *rb);
static size_t size(ring_buffer_t *rb);

ring_buffer_t *ring_buffer_init(ring_buffer_t *rb, void *buffer,
    size_t length, int flags)
{
    if(NULL == rb) return NULL;

    rb->wr_head = 0;
    rb->rd_head = 0;
    rb->flags   = flags;
    rb->length  = length;
    rb->buffer  = buffer;

    return rb;
}

int ring_buffer_push(ring_buffer_t *rb, char c)
{
    char *ptr;

    if(NULL == rb)
        return -RING_BUFFER_EINVALID;
    if(full(rb) && !(rb->flags & RING_BUFFER_OVERWRITE))
        return -RING_BUFFER_EFULL;

    ptr = (char*)rb->buffer;
    ptr[mask(rb, rb->wr_head)] = c;
    rb->wr_head++;

    return RING_BUFFER_ESUCCESS;
}

int ring_buffer_pop(ring_buffer_t *rb, char *c)
{
    char *ptr;

    if(NULL == rb)
        return -RING_BUFFER_EINVALID;
    if(empty(rb))
        return -RING_BUFFER_EEMPTY;

    ptr = (char*)rb->buffer;
    *c = ptr[mask(rb, rb->rd_head)];
    rb->rd_head++;

    return RING_BUFFER_ESUCCESS;
}

static inline size_t mask(ring_buffer_t *rb, size_t index)
{
    return index % rb->length;
}

static inline bool empty(ring_buffer_t *rb)
{
    return rb->rd_head == rb->wr_head;
}

static inline bool full(ring_buffer_t *rb)
{
    return size(rb) == rb->length;
}

static inline size_t size(ring_buffer_t *rb)
{
    return rb->rd_head - rb->wr_head;
}