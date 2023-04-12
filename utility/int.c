#include "int.h"

void print_int(const void *x) {
    printf("%d", *((int *) x));
}

size_t hash_int(const void *key) {
    const int *p = key;
    size_t x = *p;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}


int comparator_int(const void *x, const void *y) {
    const int *a = x;
    const int *b = y;
    int diff = *a - *b;
    if (diff == 0) return 0;
    else if (diff > 0) return 1;
    else return -1;
}