#include "utility/utility.h"
#include "stack.h"

struct Stack *create_stack(size_t element_size) {
    struct Stack *st = malloc_safe(sizeof(struct Stack));
    st->element_size = element_size;
    st->size = 0;
    struct LinkedList *ll = create_linked_list(element_size, NULL);
    st->linked_list = ll;
    return st;
}

const void *peek_stack(struct Stack *st) {
    struct LinkedList *ll = st->linked_list;
    const void *data = get_from_linked_list(ll, 0);
    return data == NULL ? NULL : data;
}

error pop_stack(struct Stack *st, void *address) {
    struct LinkedList *ll = st->linked_list;
    error e = remove_first_from_linked_list(ll, address);
    if (e == ERROR_OK) {
        st->size--;
        return e;
    }
    return ERROR_EMPTY_STACK;
}

// Pushes a copy of item pointed to by data to the stack
error push_stack(struct Stack *st, const void *data) {
    struct LinkedList *ll = st->linked_list;
    add_first_to_linked_list(ll, data);
    st->size++;
    return ERROR_OK;
}

void print_stack(struct Stack *st, print_element_ptr printer) {
    struct LinkedList *ll = st->linked_list;
    print_linked_list(ll, printer);
}
