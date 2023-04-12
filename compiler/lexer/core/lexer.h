#ifndef COMPILER_LEXER_H
#define COMPILER_LEXER_H

#include "datastructures/hashmap/hashmap.h"
#include "compiler/symbol_table/interface/symbol_table.h"
#include <stdio.h>

#define DEFAULT_LEXER_BUFFER_SIZE 1024
#define MAX_LEXEME_LENGTH 20
#define MAX_TOKEN_SIZE 16
#define DOLlAR '$'

typedef struct Lexer {
    char *file_name;
    Token *current_token;
    FILE *file;
    bool returned_dollar_token;
    int buffer_size;
    char *buffer1;
    char *buffer2;
    int buffer_number;
    int forward;
    int current_line;
    HashMap *string_to_token_type_map;
    SymbolTable *symbol_table;
    struct DFA *dfa;
} Lexer;

void destroy_lexer(Lexer *lexer);

void print_lexer(Lexer *lexer, FILE *fp);

Token *get_next_token(Lexer *lexer);

Token *get_current_token(Lexer *lexer);

void set_string_to_token_type_map_for_lexer(Lexer *lexer, HashMap *string_to_token_type_map);

void run_lexer_independently_on_and_print_to(Lexer *lexer, FILE *out);

void print_token(Lexer *lexer, Token *t);

void grant_symbol_table_to_lexer(Lexer *lexer, SymbolTable *symbol_table);


#endif //COMPILER_LEXER_H
