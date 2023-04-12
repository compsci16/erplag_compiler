#include <stdlib.h>
#include <stdio.h>
#include "hashset.h"
#include "utility/utility.h"
#include "datastructures/hash/hash.h"

static size_t hash(HashSet *hashset, const void *data);

// TODO: Implement shrink_buckets
static void grow_buckets(HashSet *hashset);

static void set_next_iterable_bucket_if_it_exists(HashSetIterator *iterator);

static double get_load_factor(HashSet *hashset);

static void initialize_buckets(struct HashSet *hashset, struct LinkedList **buckets, size_t size);

bool remove_from_hashset(HashSet *hashset, const void *data) {
    return remove_from_list(hashset->buckets[hash(hashset, data)], data);
}

HashSet *clone(HashSet *from) {
    HashSet *cloned = create_hashset(from->element_size, from->hash_function_ptr,
                                     from->comparator);
    add_all_from_hashset(from, cloned);
    return cloned;
}

struct HashSet *create_hashset(size_t element_size, hash_function_ptr hash_function_ptr, comparator comparator) {
    HashSet *hashset = malloc_safe(sizeof(struct HashSet));
    hashset->hash_function_ptr = hash_function_ptr;
    hashset->element_size = element_size;
    hashset->num_items = 0;
    hashset->num_buckets = INITIAL_HASHSET_BUCKETS_SIZE;
    hashset->buckets = malloc_safe(hashset->num_buckets * sizeof(struct linked_list *));
    hashset->comparator = comparator;
    initialize_buckets(hashset, hashset->buckets, hashset->num_buckets);
    return hashset;
}

void destroy_hashset(HashSet *hashset) {
    for (int i = 0; i < hashset->num_buckets; ++i) {
        destroy_linked_list(hashset->buckets[i]);
    }
    free(hashset->buckets);
    free(hashset);
}

error hashset_union_with(HashSet *hashset_to_get_all, const HashSet *hashset_to_merge) {
    add_all_from_hashset(hashset_to_merge, hashset_to_get_all);
    return ERROR_OK;
}

struct HashSet *hashset_union(const HashSet *hashset1, const HashSet *hashset2) {
    struct HashSet *combined = create_hashset(hashset1->element_size, hashset1->hash_function_ptr,
                                              hashset1->comparator);
    add_all_from_hashset(hashset1, combined);
    add_all_from_hashset(hashset2, combined);
    return combined;
}

void add_all_from_hashset(const HashSet *from, HashSet *to) {
    for (int i = 0; i < from->num_buckets; ++i) {
        size_t chain_size = from->buckets[i]->size;
        for (size_t j = 0; j < chain_size; ++j) {
            const void *x = get_from_linked_list(from->buckets[i], j);
            add_to_hashset(to, x);
        }
    }
}

// TODO: No need to pass buckets
// TODO: No need to pass size as hashshet has num of buckets
static void initialize_buckets(HashSet *hashset, struct LinkedList **buckets, size_t size) {
    for (int i = 0; i < size; ++i) {
        buckets[i] = create_linked_list(hashset->element_size, hashset->comparator);
    }
}

error add_to_hashset(HashSet *hashset, const void *data) {
    if (get_load_factor(hashset) >= RESIZE_LOAD_FACTOR) {
        grow_buckets(hashset);
    }

    struct LinkedList *chain = hashset->buckets[hash(hashset, data)];
    if (linked_list_contains(chain, data)) {
        return ERROR_ALREADY_PRESENT;
    }
    add_first_to_linked_list(chain, data);
    hashset->num_items++;
    return ERROR_OK;
}

bool hashset_contains(HashSet *hashset, const void *data) {
    struct LinkedList *chain = hashset->buckets[hash(hashset, data)];
    return linked_list_contains(chain, data);
}

static double get_load_factor(HashSet *hashset) {
    return (double) hashset->num_items / (double) hashset->num_buckets;
}

bool is_empty_set(HashSet *hashset) {
    return hashset->num_items == 0;
}

size_t get_size_hashset(HashSet *hashset) {
    return hashset->num_items;
}

static void grow_buckets(HashSet *hashset) {
    struct LinkedList **old_buckets = hashset->buckets;
    size_t old_buckets_size = hashset->num_buckets;
    size_t new_buckets_size = old_buckets_size * 2;
    hashset->num_buckets = new_buckets_size;
    struct LinkedList **new_buckets = malloc_safe(new_buckets_size * sizeof(struct linked_list *));
    hashset->buckets = new_buckets;
    initialize_buckets(hashset, new_buckets, new_buckets_size);

    for (int i = 0; i < old_buckets_size; ++i) {
        struct LinkedList *bucket = old_buckets[i];
        size_t initial_size = get_size_of_linked_list(bucket);
        for (int j = 0; j < initial_size; ++j) {
            void *data = malloc_safe(hashset->element_size);
            remove_first_from_linked_list(bucket, data);
            size_t new_bucket_number = hash(hashset, data);
            add_first_to_linked_list(hashset->buckets[new_bucket_number], data);
            free(data);
        }
        destroy_linked_list(bucket);
    }
    free(old_buckets);
}

size_t hash(HashSet *hashset, const void *data) {
    size_t hash = hashset->hash_function_ptr(data);
    return hash % hashset->num_buckets;
}

void print_hashset_detailed(HashSet *hashset, print_element_ptr printer) {
    puts("Printing Hashset Buckets with their chains: ");
    for (int i = 0; i < hashset->num_buckets; ++i) {
        printf("%d.", i);
        printf(" Bucket size = %zu", get_size_of_linked_list(hashset->buckets[i]));
        print_linked_list(hashset->buckets[i], printer);
        puts("");
    }
}

void print_all_keys_hashset(HashSet *hashset, print_element_ptr printer) {
    for (int i = 0; i < hashset->num_buckets; ++i) {
        LinkedList *b = hashset->buckets[i];
        size_t size = get_size_of_linked_list(b);
        if (size > 0) {
            LinkedListNode *current = b->sentinel->next;
            for (int j = 0; j < size; ++j) {
                printer(current->data);
                printf(", ");
                current = current->next;
            }
        }
    }
    puts("");
}

HashSetIterator *create_hashset_iterator(HashSet *to_iterate) {
    HashSetIterator *iterator = malloc_safe(sizeof(struct HashSetIterator));
    iterator->hs = to_iterate;
    iterator->bucket_number = 0;
    set_next_iterable_bucket_if_it_exists(iterator);
    return iterator;
}

const void *get_next_hashset(HashSetIterator *iterator) {
    if (iterator->current_linked_list == NULL) return NULL;
    void *to_return = iterator->current_bucket_node->data;
    iterator->index_in_linked_list++;
    iterator->current_bucket_node = iterator->current_bucket_node->next;
    if (iterator->index_in_linked_list == get_size_of_linked_list(iterator->current_linked_list)) {
        iterator->bucket_number++;
        set_next_iterable_bucket_if_it_exists(iterator);
    }
    return to_return;
}

static void set_next_iterable_bucket_if_it_exists(HashSetIterator *iterator) {
    int bucket_number = iterator->bucket_number;
    HashSet *to_iterate = iterator->hs;
    while (bucket_number < to_iterate->num_buckets && is_empty_linked_list(to_iterate->buckets[bucket_number])) {
        ++bucket_number;
    }
    if (bucket_number == to_iterate->num_buckets) {
        iterator->current_linked_list = NULL;
        iterator->current_bucket_node = NULL;
        iterator->index_in_linked_list = 0;
        iterator->bucket_number = -1;
        return;
    }
    iterator->bucket_number = bucket_number;
    iterator->current_linked_list = to_iterate->buckets[bucket_number];
    iterator->current_bucket_node = iterator->current_linked_list->sentinel->next;
    iterator->index_in_linked_list = 0;
}

void destroy_hashset_iterator(HashSetIterator *iterator) {
    free(iterator);
}