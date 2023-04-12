#ifndef COMPILER_CFG_H
#define COMPILER_CFG_H

#include "datastructures/hashset/hashset.h"
#include "datastructures/hashmap/hashmap.h"

/*
 * Struct for Grammar with terminals as integers ranging from TERMINALS_BEGIN to TERMINALS_END
 *                         non-terminals as integers ranging from NON_TERMINALS_BEGIN to NON_TERMINALS_END
 *                         the two ranges are continuous in [0, N_NON_TERMINALS + N_TERMINALS] and mutually exclusive
 *
 *  symbol to string map hence contains the string for the corresponding terminal o rnon temrinal at the index
 */


typedef struct Production {
    int lhs;
    int rhs_size;
    int rhs[];
} Production;

typedef struct Grammar {
    Production **productions;
    int N_PRODUCTIONS;
    HashMap *string_to_terminal_map;
    HashMap *string_to_non_terminal_map;
    HashMap *string_to_action_map;
    HashMap *first_sets_map;
    HashMap *follow_sets_map;
} Grammar;

void print_symbol(const void *symbol);

void print_production(const Grammar *g, Production *p);

bool is_null_production(Production *p);

char *get_production_string(Grammar *g, Production *p);

#endif //COMPILER_CFG_H
