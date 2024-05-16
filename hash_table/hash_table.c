#include "hash_table.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

// Return 64-bit FNV-1a hash for key (NUL-terminated). See description:
// https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function
static uint64_t hash_function(const char* key) {
    uint64_t hash = FNV_OFFSET;
    for (const char* p = key; *p; p++) {
        hash ^= (uint64_t)(unsigned char)(*p);
        hash *= FNV_PRIME;
    }
    return hash;
}

hash_table init_hash_table(int capacity, size_t value_size) {
    hash_table_row *rows = calloc(capacity, sizeof(hash_table_row));
    hash_table table = {
        .rows = rows,
        .capacity = capacity,
        .size = 0,
        .value_size = value_size
    };
    return table;
}

void clean_hash_table(hash_table *ht) {
    for (int i = 0; i < ht->capacity; ++i) {
        free(ht->rows[i].key);
        free(ht->rows[i].value);
    }
    free(ht->rows);
}

char hash_table_set(hash_table *ht, char *key, void *value) {
    if (ht->capacity == ht->size) {
        hash_table_row *new_rows = calloc(ht->capacity * 2, sizeof(hash_table_row));
        memcpy(new_rows, ht->rows, ht->capacity * sizeof(hash_table_row));
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
    return 1;
}

void* hash_table_get(hash_table *ht, char *key) {
    size_t row_index = hash_function(key) & (ht->capacity - 1);
    while(ht->rows[row_index].key != NULL) {
        if (strcmp(ht->rows[row_index].key, key) == 0) {
            return ht->rows[row_index].value;
        }
        row_index = (row_index + 1) % ht->capacity;
    }
    return NULL;
}