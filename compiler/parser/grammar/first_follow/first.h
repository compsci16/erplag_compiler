#ifndef COMPILER_FIRST_H
#define COMPILER_FIRST_H

#include "datastructures/hashset/hashset.h"
#include "datastructures/hashmap/hashmap.h"
#include "grammar/cfg.h"

error set_first_sets_for(Grammar *g);

HashSet *first_set_of_symbols(const Grammar *g, int symbols[], int size);

#endif //COMPILER_FIRST_H
