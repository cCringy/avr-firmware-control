#include "buffer.h"
#include <stdlib.h>

struct buffer
{
    uint16_t buffer_size;
    uint16_t buffer_head;
    uint16_t buffer_tail;
    uint16_t buffer_len;
    uint32_t overflow_count;
    item_t* data;
};

buffer_t buffer_create(uint16_t size)

{
    buffer_t buffer = (buffer_t) malloc(sizeof(struct buffer));
    buffer->buffer_size = size;
    buffer->buffer_head = 0;
    buffer->buffer_tail = 0;
    buffer->buffer_len = 0;
    buffer->overflow_count = 0;
    buffer->data = (item_t*) malloc(size * sizeof(item_t));
    return buffer;
}
void buffer_delete(buffer_t* buffer)
{
    if(!buffer || *buffer == NULL)
        return;
    free((*buffer)->data);
    free((*buffer));
    *buffer = NULL;
}
inline uint16_t buffer_getlen(buffer_t buffer)
{
    if (!buffer)
        return 0;
    
    return buffer->buffer_len;
}
inline uint32_t buffer_get_overflow_count(buffer_t buffer)
{
    if (!buffer)
        return 0;
    
    return buffer->overflow_count;
}
inline void buffer_insert(buffer_t buffer, item_t value)
{
    if (!buffer)
        return;
    
    buffer->data[buffer->buffer_head] = value;
    buffer->buffer_head = (buffer->buffer_head + 1) % buffer->buffer_size;
    if (buffer->buffer_len == buffer->buffer_size) {
        // buffer is full, overwrite oldest data
        buffer->buffer_tail = (buffer->buffer_tail + 1) % buffer->buffer_size;
        buffer->overflow_count++;
    } else {
        buffer->buffer_len++;
    }
}
item_t buffer_pop(buffer_t buffer)
{
    if (!buffer || buffer->buffer_len == 0)
        return INVALID_VAL;
    
    item_t value = buffer->data[buffer->buffer_tail];
    buffer->buffer_tail = (buffer->buffer_tail + 1) % buffer->buffer_size;
    buffer->buffer_len--;
    return value;
}
inline item_t buffer_peek(buffer_t buffer, uint16_t age)
{
    if (!buffer || age >= buffer_getlen(buffer))
        return INVALID_VAL;
    
    return buffer->data[(buffer->buffer_head + buffer->buffer_size - (age + 1)) % buffer->buffer_size];
}