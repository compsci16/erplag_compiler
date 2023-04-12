#ifndef COMPILER_HASHMAP_H
#define COMPILER_HASHMAP_H

#include <stddef.h>
#include "datastructures/hashset/hashset.h"
#include "datastructures/hash/hash.h"

#define INITIAL_HASHMAP_BUCKETS_SIZE 2
#define RESIZE_LOAD_FACTOR 0.75


typedef struct Entry {
    void *key;
    void *value;
} Entry;


typedef struct HashMap {
    size_t num_buckets;
    size_t num_items;
    size_t key_size;
    size_t value_size;
    size_t element_size;
    hash_function_ptr hash_function_ptr;
    comparator key_comparator;
    struct LinkedList **buckets;
} HashMap;

HashMap *create_hashmap(size_t key_size, size_t value_size, hash_function_ptr hash_function_ptr, comparator comparator);

size_t get_size_hashmap(HashMap *map);

bool hashmap_contains(const HashMap *map, const void *key);

void *get_from_hashmap(const HashMap *map, const void *key);

error put_in_hashmap(HashMap *map, const void *key, const void *value);

bool is_empty_hashmap(HashMap *map);

void print_hashmap_detailed(HashMap *map, print_element_ptr key_printer, print_element_ptr val_printer);

void print_all_key_val_pairs_hashmap(HashMap *map, print_element_ptr key_printer, print_element_ptr val_printer);

void destroy_hashmap(HashMap *map);

#endif //COMPILER_HASHMAP_H
