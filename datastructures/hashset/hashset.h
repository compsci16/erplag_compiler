#ifndef COMPILER_HASHSET_H
#define COMPILER_HASHSET_H

#include <stddef.h>
#include <stdbool.h>
#include "utility/errors.h"
#include "datastructures/linkedlist/linkedlist.h"
#include "datastructures/hash/hash.h"


#define INITIAL_HASHSET_BUCKETS_SIZE 2
#define RESIZE_LOAD_FACTOR 0.75

typedef struct HashSet {
    size_t num_buckets;
    size_t num_items;
    size_t element_size;
    hash_function_ptr hash_function_ptr;
    comparator comparator;
    struct LinkedList **buckets;
} HashSet;

typedef struct HashSetIterator {
    HashSet *hs;
    int bucket_number;
    LinkedListNode *current_bucket_node;
    int index_in_linked_list;
    LinkedList *current_linked_list;
} HashSetIterator;

HashSetIterator *create_hashset_iterator(HashSet *to_iterate);

const void *get_next_hashset(HashSetIterator *iterator);

void destroy_hashset_iterator(HashSetIterator *iterator);

void print_hashset_detailed(HashSet *hashset, print_element_ptr printer);

// adds all elements of hashset from to hashset to.
void add_all_from_hashset(const HashSet *from, struct HashSet *to);

void print_all_keys_hashset(HashSet *hashset, print_element_ptr printer);

HashSet *create_hashset(size_t element_size, hash_function_ptr hash_function_ptr, comparator comparator);

error add_to_hashset(HashSet *hashset, const void *data);

bool hashset_contains(HashSet *hashset, const void *data);

// Returns a pointer to a set containing the union of the two hashsets args
struct HashSet *hashset_union(const HashSet *hashset1, const HashSet *hashset2);

// hashset_to_get_all gets all elements from the second hashset argument
error hashset_union_with(HashSet *hashset_to_get_all, const HashSet *hashset_to_merge);

HashSet *clone(HashSet *from);

void destroy_hashset(HashSet *hashset);

bool remove_from_hashset(HashSet *hashset, const void *data);

bool is_empty_set(HashSet *hashset);

size_t get_size_hashset(HashSet *hashset);


#endif //COMPILER_HASHSET_H
