#ifndef __STACK__
#define __STACK__


#include <stdlib.h>
#include <stdint.h>

typedef struct {
    void *buffer; 
    size_t capacity; 
    size_t length;
    size_t element_size;
} Stack;

Stack init_stack(size_t capacity, size_t element_size);

void delete_stack(Stack *st);

char stack_push(Stack *st, void *value);

void stack_pop(Stack *st);

void *stack_top(Stack *st);

void const *stack_at(Stack *st, size_t offset);

#endif