#include "linkedlist.h"
#include "utility/utility.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct LinkedList *create_linked_list(size_t element_size, comparator comparator) {
    struct LinkedList *ll = malloc_safe(sizeof(struct LinkedList));
    ll->element_size = element_size;
    ll->size = 0;
    struct LinkedListNode *sentinel = malloc_safe(sizeof(struct LinkedListNode));
    sentinel->data = NULL;
    sentinel->next = sentinel;
    sentinel->previous = sentinel;
    ll->sentinel = sentinel;
    ll->comparator = comparator;
    return ll;
}

bool is_empty_linked_list(LinkedList *list) {
    return list->size == 0;
}

error add_last_to_linked_list(struct LinkedList *linked_list, const void *data) {
    struct LinkedListNode *node = malloc_safe(sizeof(struct LinkedListNode));
    node->data = malloc_safe(linked_list->element_size);
    memcpy(node->data, data, linked_list->element_size);
    struct LinkedListNode *old_tail = linked_list->sentinel->previous;
    old_tail->next = node;
    linked_list->sentinel->previous = node;
    node->previous = old_tail;
    node->next = linked_list->sentinel;
    linked_list->size++;
    return ERROR_OK;
}

bool linked_list_contains(struct LinkedList *linked_list, const void *data) {
    struct LinkedListNode *current = linked_list->sentinel->next;
    for (size_t i = 0; i < linked_list->size; ++i) {
        struct LinkedListNode *next = current->next;
        if (linked_list->comparator(current->data, data) == 0) {
            return true;
        }
        current = next;
    }
    return false;
}

error update_at_index_linked_list(LinkedList *list, size_t index, const void *updated_data) {
    if (index >= list->size || index < 0) {
        return ERROR_INVALID_INDEX;
    }
    LinkedListNode *current = list->sentinel->next;
    for (size_t i = 0; i < index; ++i) {
        current = current->next;
    }
    free(current->data);
    current->data = malloc_safe(list->element_size);
    memcpy(current->data, updated_data, list->element_size);
    return ERROR_OK;
}

// Removes data from the linked list in O(n) time if it exists

bool remove_from_list(LinkedList *linked_list, const void *data) {
    LinkedListNode *current = linked_list->sentinel->next;
    size_t size = linked_list->size;
    for (size_t i = 0; i < size; ++i) {
        if (linked_list->comparator(data, current->data) == 0) {
            LinkedListNode *previous_to_remove = current->previous;
            previous_to_remove->next = current->next;
            current->next->previous = previous_to_remove;
            linked_list->size--;
            free(current->data);
            free(current);
            return true;
        }
        current = current->next;
    }
    return false;
}

error add_first_to_linked_list(struct LinkedList *linked_list, const void *data) {
    return add(linked_list, data, 0);
}

error add(struct LinkedList *linked_list, const void *data, size_t index) {
    if (index > linked_list->size) {
        return ERROR_INVALID_INDEX;
    }
    if (index == linked_list->size) {
        return add_last_to_linked_list(linked_list, data);
    }
    struct LinkedListNode *to_insert = malloc_safe(sizeof(struct LinkedListNode));
    to_insert->data = malloc_safe(linked_list->element_size);
    memcpy(to_insert->data, data, linked_list->element_size);

    struct LinkedListNode *tracker = linked_list->sentinel;
    for (size_t i = 0; i < index; ++i) {
        tracker = tracker->next;
    }
    struct LinkedListNode *temp = tracker->next;
    tracker->next = to_insert;
    to_insert->next = temp;
    to_insert->previous = tracker;
    temp->previous = to_insert;
    linked_list->size++;
    return ERROR_OK;
}

const void *get_from_linked_list(struct LinkedList *linked_list, size_t index) {
    if (index >= linked_list->size) {
        return NULL;
    }
    struct LinkedListNode *tracker = linked_list->sentinel->next;
    for (size_t i = 0; i < index; ++i) {
        tracker = tracker->next;
    }
    return tracker->data;
}

void destroy_linked_list(struct LinkedList *linked_list) {
    struct LinkedListNode *current = linked_list->sentinel;
    for (size_t i = 0; i < linked_list->size; ++i) {
        struct LinkedListNode *next = current->next;
        free(current->data);
        free(current);
        current = next;
    }
    free(linked_list);
}

size_t get_size_of_linked_list(struct LinkedList *list) {
    return list->size;
}

void print_linked_list(struct LinkedList *linked_list, print_element_ptr printer) {
    if (!linked_list) return;
    if (linked_list->size == 0) return;
    struct LinkedListNode *current = linked_list->sentinel->next;
    for (size_t i = 0; i < linked_list->size; ++i) {
        printf("%s", " => ");
        printer(current->data);
        current = current->next;
    }
}

error remove_first_from_linked_list(struct LinkedList *linked_list, void *address) {
    if (linked_list->size == 0) {
        return ERROR_EMPTY_LIST;
    }
    struct LinkedListNode *old_head = linked_list->sentinel->next;
    memcpy(address, old_head->data, linked_list->element_size);
    linked_list->sentinel->next = old_head->next;
    old_head->next->previous = linked_list->sentinel;

    free(old_head->data);
    free(old_head);

    linked_list->size--;
    return ERROR_OK;
}
