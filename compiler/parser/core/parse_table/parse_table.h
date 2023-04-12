#ifndef COMPILER_PARSE_TABLE_H
#define COMPILER_PARSE_TABLE_H

#include <stdlib.h>
#include "grammar/cfg.h"

Production ***get_parse_table_for(const Grammar *g);

void print_parse_table(Grammar *g, Production ***table, char *file);

void destroy_parse_table(const Grammar *g, Production ***parse_table);

#endif //COMPILER_PARSE_TABLE_H
