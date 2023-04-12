#ifndef COMPILER_INT_H
#define COMPILER_INT_H

#include <stdio.h>
#include <stdbool.h>

void print_int(const void *x);

size_t hash_int(const void *key);

int comparator_int(const void *x,const void *y);

#endif //COMPILER_INT_H
