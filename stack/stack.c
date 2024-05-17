#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "stack.h"


Stack init_stack(size_t capacity, size_t element_size) {
    Stack st = {
        .buffer = malloc(capacity * element_size),
        .capacity = capacity,
        .element_size = element_size,
        .length = 0
    };
    return st;
}

void delete_stack(Stack *st) {
    free(st->buffer);
    st->buffer = NULL;
}

char stack_push(Stack *st, void *value) {
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

void stack_pop(Stack *st) {
    if (st->length == 0) return;

    st->length -= 1; 
}

void *stack_top(Stack *st) {
    return (char *)st->buffer + (st->length-1)*st->element_size;
}

void const *stack_at(Stack *st, size_t offset) {
    if (offset >= st->length) return NULL;
    return (char *)st->buffer + (st->length-1-offset)*st->element_size;
}