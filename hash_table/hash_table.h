#include <stdint.h>
#include <stdlib.h>

typedef struct {
    char *key;
    void *value;
} hash_table_row;

typedef struct {
    hash_table_row *rows; 
    size_t capacity; 
    size_t size;
    size_t value_size;
} hash_table; 

hash_table init_hash_table(size_t capacity, size_t value_size);
void clean_hash_table(hash_table *ht);

char hash_table_set(hash_table *ht, char *key, void *value);
void* hash_table_get(hash_table *ht, char *key);