#ifndef COMPILER_UTILITY_H
#define COMPILER_UTILITY_H

#include <stddef.h>

//#define RED   "\x1B[31m"
//#define GRN   "\x1B[32m"
//#define YEL   "\x1B[33m"
//#define BLU   "\x1B[34m"
//#define MAG   "\x1B[35m"
//#define CYN   "\x1B[36m"
//#define WHT   "\x1B[37m"
//#define RESET "\x1B[0m"

void *malloc_safe(size_t n);

char *get_upper(const char *str);

void *realloc_safe(void *ptr, size_t size);

void *calloc_safe(size_t count, size_t size);

#endif //COMPILER_UTILITY_H
