#include "grammar/grammar_populator/constructor.h"
#include "compiler/symbol_table/constructor/symbol_table_constructor.h"
#include "parser.h"
#include "lexer_constructor.h"
#include "core/ast/ast.h"

char *state_to_token_path = "files/lexer_files/id_to_token_type.txt";
char *edges_path = "files/lexer_files/edges.txt";
char *ac_retract_path = "files/lexer_files/accept_retract_other.txt";
char *state_to_error = "files/lexer_files/state_to_error.txt";

int main() {
    const Grammar *g = get_initialized_grammar("/Users/wint/code/compiler/files/grammar_files/mini_productions.txt");
    char *src = "/Users/wint/code/compiler/files/testcases/semanicAnalysis_testcases/t1.txt";
    int buffer_size = 1024;
    Parser *parser = get_initialized_parser(g);
    Lexer *lexer = get_initialized_lexer_for_parser(parser, src, state_to_token_path, edges_path,
                                                    ac_retract_path,
                                                    state_to_token_path, state_to_error, buffer_size);
    parser->lexer = lexer;
    set_string_to_token_type_map_for_lexer(lexer, parser->grammar->string_to_terminal_map);

    initialize_parser_symbol_table(parser);
    // print_symbol_table(interface->symbol_table);
    grant_symbol_table_to_lexer(parser->lexer, parser->symbol_table);
    get_parse_tree(parser);
    // run_lexer_independently_on_and_print_to(lexer, stdout);
    printf("Parse Tree Number of Nodes = %d, Allocated Memory = %d\n", parse_tree_number_of_nodes,
           parse_tree_allocated_memory);
    printf("AST Number of Nodes = %d, Allocated Memory = %d\n", ast_number_of_nodes,
           ast_allocated_memory);
    double compression_percentage =
            ((double) (parse_tree_allocated_memory - ast_allocated_memory)) / ((double) (parse_tree_allocated_memory));
    compression_percentage = compression_percentage * 100;
    printf("Compression Percentage = %lf\n", compression_percentage);
    return 0;
}
