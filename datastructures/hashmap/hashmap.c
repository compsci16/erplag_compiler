#include "hashmap.h"
#include "utility/utility.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
* Returns the index of the first occurrence of key in bucket; SIZE_MAX if not present
 */
size_t index_in_internal_linked_list_of_key(const HashMap *map, size_t bucket_number, const void *key);

static void grow_buckets(HashMap *map);

static double get_load_factor(const HashMap *map);

static size_t hash(const HashMap *map, const void *key);

static void
print_bucket_list(HashMap *map, size_t bucket_number, print_element_ptr key_printer, print_element_ptr val_printer);

static void initialize_buckets(HashMap *map);

HashMap *
create_hashmap(size_t key_size, size_t value_size, hash_function_ptr hash_function_ptr, comparator comp) {
    HashMap *map = malloc_safe(sizeof(struct HashMap));
    map->hash_function_ptr = hash_function_ptr;
    map->key_size = key_size;
    map->value_size = value_size;
    map->num_buckets = INITIAL_HASHMAP_BUCKETS_SIZE;
    map->buckets = malloc_safe(map->num_buckets * sizeof(struct LinkedList *));
    map->key_comparator = comp;
    map->num_items = 0;
    map->element_size = sizeof(struct Entry);
    initialize_buckets(map);
    return map;
}

static void initialize_buckets(HashMap *map) {
    LinkedList **buckets = map->buckets;
    size_t size = map->num_buckets;
    for (size_t i = 0; i < size; ++i) {
        /* As we are not relying on the linked list's comparing functionality in this hashmap due to wrapper key-value
         struct */
        buckets[i] = create_linked_list(map->element_size, NULL);
    }
}

size_t get_size_hashmap(HashMap *map) {
    return map->num_items;
}

bool hashmap_contains(const HashMap *map, const void *key) {
    if (index_in_internal_linked_list_of_key(map, hash(map, key), key) == SIZE_MAX) {
        return false;
    }
    return true;
}

size_t index_in_internal_linked_list_of_key(const HashMap *map, size_t bucket_number, const void *key) {
    LinkedList *bucket = map->buckets[bucket_number];
    struct LinkedListNode *current = bucket->sentinel->next;

    size_t ll_size = get_size_of_linked_list(bucket);
    for (size_t i = 0; i < ll_size; ++i) {
        Entry *e = current->data;
        void *k = e->key;
        if (map->key_comparator(k, key) == 0) {
            return i;
        }
        current = current->next;
    }
    return SIZE_MAX;
}

static size_t hash(const HashMap *map, const void *key) {
    size_t hash = map->hash_function_ptr(key);
    return hash % map->num_buckets;
}

void *get_from_hashmap(const HashMap *map, const void *search_key) {
    if (!hashmap_contains(map, search_key)) {
        return NULL;
    }
    LinkedList *list_to_search = map->buckets[hash(map, search_key)];
    LinkedListNode *node = list_to_search->sentinel->next;
    for (size_t i = 0; i < list_to_search->size; ++i) {
        Entry *e = node->data;
        if (map->key_comparator(e->key, search_key) == 0) {
            return e->value;
        }
        node = node->next;
    }
    return NULL;
}

error put_in_hashmap(HashMap *map, const void *key, const void *value) {
    if (get_load_factor(map) >= RESIZE_LOAD_FACTOR) {
        grow_buckets(map);
    }

    size_t bucket_number = hash(map, key);
    size_t index_in_bucket = index_in_internal_linked_list_of_key(map, bucket_number, key);

    Entry *e = malloc_safe(sizeof(struct Entry));
    e->key = malloc_safe(map->key_size);
    e->value = malloc_safe(map->element_size);
    memcpy(e->key, key, map->key_size);
    memcpy(e->value, value, map->value_size);

    if (index_in_bucket != SIZE_MAX) {
        // key already present, update entry in bucket's list
        return update_at_index_linked_list(map->buckets[bucket_number], index_in_bucket, e);
    }

    error err = add_first_to_linked_list(map->buckets[hash(map, key)], e);
    if (err == ERROR_OK) {
        map->num_items++;
    }
    return err;
}

bool is_empty_hashmap(HashMap *map) {
    return map->num_items == 0;
}

static double get_load_factor(const HashMap *map) {
    return (double) map->num_items / (double) map->num_buckets;
}

static void grow_buckets(HashMap *map) {
    LinkedList **old_buckets = map->buckets;
    size_t old_buckets_size = map->num_buckets;
    size_t new_buckets_size = map->num_buckets * 2;
    map->num_buckets = new_buckets_size;
    LinkedList **new_buckets = malloc_safe(sizeof(LinkedList *) * map->num_buckets);
    map->buckets = new_buckets;
    initialize_buckets(map);
    for (size_t i = 0; i < old_buckets_size; ++i) {
        LinkedList *current_bucket = old_buckets[i];
        while (!is_empty_linked_list(current_bucket)) {
            Entry *entry_to_remap = malloc_safe(sizeof(Entry));
            remove_first_from_linked_list(current_bucket, entry_to_remap);
            size_t new_bucket_number = hash(map, entry_to_remap->key);
            add_first_to_linked_list(map->buckets[new_bucket_number], entry_to_remap);
            free(entry_to_remap);
        }
        destroy_linked_list(current_bucket);
    }
    free(old_buckets);
}

void print_hashmap_detailed(HashMap *map, print_element_ptr key_printer, print_element_ptr val_printer) {
    for (size_t i = 0; i < map->num_buckets; ++i) {
        printf("Bucket %zu ----> ", i);
        print_bucket_list(map, i, key_printer, val_printer);
    }
}

void print_all_key_val_pairs_hashmap(HashMap *map, print_element_ptr key_printer, print_element_ptr val_printer) {
    for (size_t i = 0; i < map->num_buckets; ++i) {
        LinkedList *b = map->buckets[i];
        size_t b_size = get_size_of_linked_list(b);
        if (b_size > 0) {
            LinkedListNode *current = b->sentinel->next;
            for (size_t j = 0; j < b_size; ++j) {
                Entry *e = current->data;
                key_printer(e->key);
                printf(":");
                current = current->next;
                val_printer(e->value);
                printf(", ");
            }
        }
    }
    puts(" ");
}


static void
print_bucket_list(HashMap *map, size_t bucket_number, print_element_ptr key_printer, print_element_ptr val_printer) {
    LinkedListNode *node = map->buckets[bucket_number]->sentinel->next;
    size_t size = map->buckets[bucket_number]->size;
    for (size_t i = 0; i < size; ++i) {
        Entry *e = node->data;
        key_printer(e->key);
        printf(":");
        val_printer(e->value);
        node = node->next;
        printf(" => ");
    }
    puts(" ");
}

void destroy_hashmap(HashMap *map) {
    for (size_t i = 0; i < map->num_buckets; ++i) {
        destroy_linked_list(map->buckets[i]);
    }
    free(map->buckets);
    free(map);
}
