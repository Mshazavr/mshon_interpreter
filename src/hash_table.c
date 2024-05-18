#include "hash_table.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

// Return 64-bit FNV-1a hash for key (NUL-terminated). See description:
// https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function
static uint64_t hash_function(char const *key) {
    uint64_t hash = FNV_OFFSET;
    for (char const *p = key; *p; ++p) {
        hash ^= (uint64_t)(unsigned char)(*p);
        hash *= FNV_PRIME;
    }
    return hash;
}

HashTable init_hash_table(size_t capacity, size_t value_size) {
    HashTableRow *rows = calloc(capacity, sizeof(HashTableRow));
    HashTable table = {
        .rows = rows,
        .capacity = capacity,
        .size = 0,
        .value_size = value_size
    };
    return table;
}

void clean_hash_table(HashTable *ht) {
    for (size_t i = 0; i < ht->capacity; ++i) {
        free(ht->rows[i].key);
        free(ht->rows[i].value);
    }
    free(ht->rows);
}

char hash_table_set(HashTable *ht, char const *key, void const *value) {
    if (ht->capacity == ht->size) {
        HashTableRow *new_rows = calloc(ht->capacity * 2, sizeof(HashTableRow));
        if (new_rows == NULL) return 1;
        memcpy(new_rows, ht->rows, ht->capacity * sizeof(HashTableRow));
        ht->capacity *= 2;
    }

    size_t row_index = hash_function(key) & (ht->capacity - 1);

    while(ht->rows[row_index].key != NULL) {
        if (strcmp(ht->rows[row_index].key, key) == 0) {
            free(ht->rows[row_index].value);
            ht->rows[row_index].value = malloc(ht->value_size);
            memcpy(ht->rows[row_index].value, value, ht->value_size);
        }
        row_index = (row_index + 1) % ht->capacity;
    }

    ht->rows[row_index].key = strdup(key);
    if (ht->rows[row_index].key == NULL) return 0;
    ht->rows[row_index].value = malloc(ht->value_size);
    if (ht->rows[row_index].value == NULL) return 0;
    memcpy(ht->rows[row_index].value, value, ht->value_size);
    return 0;
}

void const * hash_table_get(HashTable const *ht, char const *key) {
    size_t row_index = hash_function(key) & (ht->capacity - 1);
    while(ht->rows[row_index].key != NULL) {
        if (strcmp(ht->rows[row_index].key, key) == 0) {
            return ht->rows[row_index].value;
        }
        row_index = (row_index + 1) % ht->capacity;
    }
    return NULL;
}