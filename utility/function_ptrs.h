#ifndef COMPILER_FUNCTION_PTRS_H
#define COMPILER_FUNCTION_PTRS_H

#include <stdbool.h>

typedef void (*print_element_ptr)(const void *);

typedef int (*comparator)(const void *, const void *);

#endif //COMPILER_FUNCTION_PTRS_H
