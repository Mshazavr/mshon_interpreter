#ifndef __HASH_TABLE__
#define __HASH_TABLE__


#include <stdint.h>
#include <stdlib.h>

typedef struct {
    char *key;
    void *value;
} HashTableRow;

typedef struct {
    HashTableRow *rows; 
    size_t capacity; 
    size_t size;
    size_t value_size;
} HashTable; 

HashTable init_hash_table(size_t capacity, size_t value_size);
void clean_hash_table(HashTable *ht);

char hash_table_set(HashTable *ht, char const *key, void const *value);
void const * hash_table_get(HashTable const *ht, char const *key);

#endif