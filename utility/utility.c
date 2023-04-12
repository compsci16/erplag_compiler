#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "utility.h"


void *malloc_safe(size_t n) {
    void *ptr = malloc(n);
    if (!ptr) {
        fprintf(stderr, "%s %zu %s", "Failed to allocate ", n, " bytes.");
        abort();
    }
    return ptr;
}

void *calloc_safe(size_t num, size_t size) {
    void *ptr = calloc(num, size);
    if (!ptr) {
        fprintf(stderr, "%s %zu*%zu %s", "Failed to allocate ", num, size, " bytes.");
        abort();
    }
    return ptr;
}

void *realloc_safe(void *ptr, size_t size) {
    void *new_ptr = realloc(ptr, size);
    if (!new_ptr) {
        fprintf(stderr, "%s %zu %s", "Failed to reallocate ", size, " bytes.");
        abort();
    }
    return new_ptr;
}

char *get_upper(const char *str) {
    int len = strlen(str);
    char *upped = malloc_safe(sizeof(char) * (len + 1));
    for (int i = 0; i < len; i++) {
        upped[i] = toupper(str[i]);
    }
    upped[len] = 0;
    return upped;
}
