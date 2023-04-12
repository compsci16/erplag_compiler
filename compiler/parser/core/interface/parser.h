#ifndef COMPILER_PARSER_CORE_XXY_H
#define COMPILER_PARSER_CORE_XXY_H

#include "grammar/cfg.h"
#include "compiler/lexer/core/lexer.h"
#include "core/parse_tree/parse_tree.h"
#include "core/parse_table/parse_table.h"

typedef struct Parser Parser;
typedef struct ParseTree ParseTree;

struct Parser {
    SymbolTable *symbol_table;
    Production ***parse_table;
    const Grammar *grammar;
    Lexer *lexer;
};

Parser *get_initialized_parser(const Grammar *grammar);

ParseTree *get_parse_tree(Parser *parser);

void initialize_parser_symbol_table(Parser *parser);

void get_symbol_table_from_parser(Parser *parser);

#endif //COMPILER_PARSER_CORE_XXY_H
