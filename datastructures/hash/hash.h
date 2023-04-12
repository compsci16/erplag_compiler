#ifndef COMPILER_HASH_H
#define COMPILER_HASH_H

#include <stddef.h>

// Generic hash function
typedef size_t (*hash_function_ptr)(const void *key);

#endif //COMPILER_HASH_H
