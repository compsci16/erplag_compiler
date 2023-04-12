#ifndef COMPILER_ARRAYLIST_H
#define COMPILER_ARRAYLIST_H

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "utility/errors.h"
#include "utility/function_ptrs.h"
#include <stdio.h>

#define INITIAL_SIZE 2
#define GROW_BY 2


typedef struct ArrayList {
    void **elements;
    size_t size;
    size_t capacity;
    size_t width;
} ArrayList;

ArrayList *create_arraylist(size_t width);

void *get_element_at_index_arraylist(ArrayList *arraylist, size_t index);

error add_element_arraylist(ArrayList *arraylist, void *data);

error remove_last_arraylist(ArrayList *arraylist, void *address);

void print_arraylist(ArrayList *arraylist, print_element_ptr printer);

size_t get_arraylist_size(ArrayList *arraylist);

void destroy_arraylist(ArrayList *arraylist);

void *get_first_arraylist(ArrayList *array_list);

bool is_empty_arraylist(ArrayList *arraylist);

#endif //COMPILER_ARRAYLIST_H



