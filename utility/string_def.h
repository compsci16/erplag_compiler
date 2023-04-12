#ifndef COMPILER_STRING_DEF_H
#define COMPILER_STRING_DEF_H

#include <string.h>
#include <stdbool.h>

int comparator_string(const void *x, const void *y);

size_t hash_string_djb2(const void *str);

void print_string(const void *x);

#endif //COMPILER_STRING_DEF_H
