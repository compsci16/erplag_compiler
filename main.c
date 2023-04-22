#include "grammar/grammar_populator/constructor.h"
#include "compiler/symbol_table/constructor/symbol_table_constructor.h"
#include "parser.h"
#include "lexer_constructor.h"
#include "core/ast/ast.h"
#include "compiler/semantic_analyzer/semantic_analyzer.h"
#include "compiler/code_generator/code_generator.h"

char *state_to_token_path = "files/lexer_files/id_to_token_type.txt";
char *edges_path = "files/lexer_files/edges.txt";
char *ac_retract_path = "files/lexer_files/accept_retract_other.txt";
char *state_to_error = "files/lexer_files/state_to_error.txt";

int main() {
    char *src = "/Users/wint/CLionProjects/erplag_compiler/files/testcases/test_code_gen/t38.txt";
    char *asm_file = "./code.asm";
    const Grammar *g = get_initialized_grammar("/Users/wint/code/compiler/files/grammar_files/productions.txt");
    Parser *parser = get_initialized_parser(g);
    Lexer *lexer = get_initialized_lexer_for_parser(parser, src, state_to_token_path, edges_path,
                                                    ac_retract_path,
                                                    state_to_token_path, state_to_error, 1024);
    parser->lexer = lexer;
    set_string_to_token_type_map_for_lexer(lexer, parser->grammar->string_to_terminal_map);
    initialize_parser_symbol_table(parser);
    grant_symbol_table_to_lexer(parser->lexer, parser->symbol_table);
    ParseTree *parse_tree = construct_parse_tree(parser);
    ProgramNode *program_node_root = parse_tree->got_syntax_error ? NULL : parse_tree->program_node;
    if (program_node_root != NULL) {
        print_ast_node((const ASTNode *) program_node_root, 0);
        construct_symbol_table(program_node_root);
        perform_semantic_analysis(program_node_root, program_node_root->global_symbol_table);
        print_symbol_table(program_node_root->global_symbol_table, 0);
        print_symbol_table_as_required(program_node_root->global_symbol_table);
        if (!GOT_SEMANTIC_ERROR_GLOBAL) {
            generate_code_from(program_node_root, asm_file);
        } else {
            puts("Failed to generate code due to Semantic Errors");
        }
    } else {
        puts("Failed to construct AST/AssemblyCode due to Syntax Errors");
    }
    return 0;
}
