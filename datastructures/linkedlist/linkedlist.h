#ifndef COMPILER_LINKEDLIST_H
#define COMPILER_LINKEDLIST_H

#include "utility/function_ptrs.h"
#include <stddef.h>
#include <stdbool.h>
#include "utility/errors.h"

typedef struct LinkedList {
    size_t element_size;
    size_t size;
    struct LinkedListNode *sentinel;
    comparator comparator;
} LinkedList;

typedef struct LinkedListNode {
    void *data;
    struct LinkedListNode *previous;
    struct LinkedListNode *next;
} LinkedListNode;


// Removes data from the linked list in O(n)
bool remove_from_list(LinkedList *linked_list, const void *data);

bool is_empty_linked_list(LinkedList *list);

error update_at_index_linked_list(LinkedList *list, size_t index, const void *data);

bool linked_list_contains(struct LinkedList *linked_list, const void *data);

struct LinkedList *create_linked_list(size_t element_size, comparator comparator);

error add(struct LinkedList *linked_list, const void *data, size_t index);

error add_last_to_linked_list(struct LinkedList *linked_list, const void *data);

error add_first_to_linked_list(struct LinkedList *linked_list, const void *data);

const void *get_from_linked_list(struct LinkedList *linked_list, size_t index);

error remove_first_from_linked_list(struct LinkedList *linked_list, void *address);

size_t get_size_of_linked_list(struct LinkedList *linked_list);

void destroy_linked_list(struct LinkedList *linked_list);

void print_linked_list(struct LinkedList *linked_list, print_element_ptr printer);

#endif //COMPILER_LINKEDLIST_H
