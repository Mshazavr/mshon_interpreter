#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "stack.h"


stack init_stack(size_t capacity, size_t element_size) {
    stack st = {
        .buffer = malloc(capacity * element_size),
        .capacity = capacity,
        .element_size = element_size,
        .length = 0
    };
    return st;
}

void delete_stack(stack *st) {
    free(st->buffer);
    st->buffer = NULL;
}

char stack_push(stack *st, void *value) {
    if (st->length == st->capacity) {
        st->capacity *= 2;
        void *new_buffer = realloc(st->buffer, st->capacity * st->element_size);
        if (new_buffer == NULL) { 
            st->capacity >>= 1;
            return 0;
        }
        st->buffer = new_buffer;
    }

    memcpy((char *)st->buffer + st->length*st->element_size, (char *)value, st->element_size);
    st->length += 1;
    return 1;
}

void stack_pop(stack *st) {
    if (st->length == 0) return;

    st->length -= 1; 
}

void const *stack_top(stack *st) {
    return (char *)st->buffer + (st->length-1)*st->element_size;
}