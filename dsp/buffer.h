#ifndef BUFFER_H_
#define BUFFER_H_

#include <inttypes.h>

#define INVALID_VAL 0xFF

// ADT pattern
typedef struct buffer buffer;   // buffer instance representation (opaque record)
typedef buffer* buffer_t;       // handle to buffer instance (opaque pointer)
typedef uint16_t item_t;        // value type stored in buffer instance

buffer_t buffer_create(uint16_t buffer_size);   // constructor
void buffer_delete(buffer_t* buffer);           // destructor

// FIFO logic
void buffer_insert(buffer_t buffer, item_t value);  // insert new value
item_t buffer_pop(buffer_t buffer);                 // return oldest value and consume it
item_t buffer_peek(buffer_t buffer, uint16_t age);  // return value with age and do not consume it

uint16_t buffer_getlen(buffer_t buffer);                // number of valid inserted values
uint32_t buffer_get_overflow_count(buffer_t buffer);    // number of overflows since creation

#endif /* BUFFER_H_ */
