#include <stdlib.h>
#include <stdint.h>

typedef struct {
    void *buffer; 
    size_t capacity; 
    size_t length;
    size_t element_size;
} stack;

stack init_stack(size_t capacity, size_t element_size);

void delete_stack(stack *st);

char stack_push(stack *st, void *value);

void stack_pop(stack *st);

void const *stack_top(stack *st);