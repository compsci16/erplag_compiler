#ifndef COMPILER_TOKEN_2_H
#define COMPILER_TOKEN_2_H


#include "lexer.h"
#include "parser.h"

Lexer *
get_initialized_lexer_for_parser(Parser *parser, char *source_code_path, char *state_to_token_path,
                                 const char *edges_path,
                                 char *accept_retract_other_path, const char *state_id_to_token_path, const char *
state_to_error_path, int size);


#endif //COMPILER_TOKEN_2_H
