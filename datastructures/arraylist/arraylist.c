#include "arraylist.h"
#include "utility/utility.h"
#include "utility/function_ptrs.h"

static void grow_arraylist(ArrayList *arraylist);

/**
 * Creates an empty list backed by an array
 */
ArrayList *create_arraylist(size_t width) {
    ArrayList *arraylist = malloc_safe(sizeof(struct ArrayList));
    arraylist->size = 0;
    arraylist->capacity = INITIAL_SIZE;
    arraylist->width = width;
    arraylist->elements = malloc_safe(sizeof(void *) * INITIAL_SIZE);
    return arraylist;
}

static void grow_arraylist(ArrayList *arraylist) {
    arraylist->capacity = arraylist->capacity * GROW_BY;
    arraylist->elements = realloc(arraylist->elements, arraylist->capacity * sizeof(void *));
}


error add_element_arraylist(ArrayList *arraylist, void *data) {
    if (arraylist->size == arraylist->capacity) {
        grow_arraylist(arraylist);
    }
    size_t size = arraylist->size;
    arraylist->elements[size] = malloc_safe(arraylist->width);
    memcpy(arraylist->elements[size], data, arraylist->width);
    arraylist->size++;
    return ERROR_OK;
}

error remove_last_arraylist(ArrayList *arraylist, void *data) {
    if (arraylist->size == 0) {
        return ERROR_EMPTY_LIST;
    }
    memcpy(data, arraylist->elements[arraylist->size - 1], arraylist->width);
    free(arraylist->elements[arraylist->size - 1]);
    arraylist->size--;
    return ERROR_OK;
}


void *get_element_at_index_arraylist(ArrayList *arraylist, size_t index) {
    if (index < 0 || index >= arraylist->size) { return NULL; }
    return arraylist->elements[index];
}


size_t get_arraylist_size(ArrayList *arraylist) {
    return arraylist->size;
}

void destroy_arraylist(ArrayList *arraylist) {
    for (size_t i = 0; i < arraylist->size; ++i) {
        free(arraylist->elements[i]);
    }
    free(arraylist->elements);
    free(arraylist);
}

void print_arraylist(ArrayList *arraylist, print_element_ptr printer) {
    for (size_t i = 0; i < arraylist->size; ++i) {
        printer(arraylist->elements[i]);
        printf(", ");
    }
    printf("\n");
}

bool is_empty_arraylist(ArrayList *array_list) {
    return array_list->size == 0;
}

void *get_first_arraylist(ArrayList *arraylist) {
    if (is_empty_arraylist(arraylist)) return NULL;
    return arraylist->elements[0];
}


