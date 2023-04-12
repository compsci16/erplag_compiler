#ifndef COMPILER_SYMBOL_TABLE_CONSRTRUCTOR_H
#define COMPILER_SYMBOL_TABLE_CONSRTRUCTOR_H
#include "ast.h"

#define REAL_SIZE 4
#define BOOLEAN_SIZE 1
#define INTEGER_SIZE 2
#define INTEGER_SIZE_STRING "2"
#define BOOLEAN_SIZE_STRING "1"

void construct_symbol_table(ProgramNode *root);
#endif //COMPILER_SYMBOL_TABLE_CONSTRUCTOR_H
