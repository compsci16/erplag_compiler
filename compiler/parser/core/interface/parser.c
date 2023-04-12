#include <ctype.h>
#include "utility/utility.h"
#include "grammar/grammar_populator/grammar_constants.h"
#include "parser.h"


static void add_all_keywords_to_ST(Parser *parser);

Parser *get_initialized_parser(const Grammar *grammar) {
    Parser *parser = malloc_safe(sizeof(Parser));
    parser->grammar = grammar;
    parser->parse_table = get_parse_table_for(grammar);
    return parser;
}

ParseTree *get_parse_tree(Parser *parser) {
    return construct_parse_tree(parser);
}

// TODO: Err handling
void initialize_parser_symbol_table(Parser *parser) {
    SymbolTable *symbol_table = create_symbol_table();
    parser->symbol_table = symbol_table;
    add_all_keywords_to_ST(parser);
}

static void add_all_keywords_to_ST(Parser *parser) {
    SymbolTable *symbol_table = parser->symbol_table;
    for (int i = 0; i < N_KEYWORDS; ++i) {
        // Token type is in all caps, so get that from keyword (which may or may not be lower case)
        SymbolTableEntry *symbol_table_entry = malloc_safe(sizeof(SymbolTableEntry));
        symbol_table_entry->name = KEYWORDS[i];
        symbol_table_entry->type_descriptor.form = TYPE_KEYWORD;
        insert_in_symbol_table(symbol_table, symbol_table_entry);
    }
}
