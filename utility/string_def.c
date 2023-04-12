#include "string_def.h"
#include <stdio.h>

// comparator function will receive pointer to string aka pointer to char*
int comparator_string(const void *x, const void *y) {
    const char *a = *((char **) x);
    const char *b = *((char **) y);
    return strcmp(a, b);
}

// hash function will receive pointer to string aka pointer to char*
size_t hash_string_djb2(const void *key) {
    char **str_ptr = (char **) key;
    const char *str = *str_ptr;
    size_t hash = 5381;
    int c;
    while (true) {
        c = (unsigned char) *str++;
        if (c == 0) return hash;
        hash = ((hash << 5) + hash) + c;
    }
}


// Print string from pointer to char*
void print_string(const void *x) {
    char **y = (char **) x;
    char *z = *y;
    printf("%s", z);
}