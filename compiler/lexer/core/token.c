#include "token.h"
#include "utility.h"
#include "grammar/grammar_populator/grammar_constants.h"

Token *create_token_with_type_and_lexeme_for(int token_type) {
    Token *token = malloc_safe(sizeof(Token));
    token->token_type = token_type;
    token->lexeme = strdup(SYMBOL_TO_STRING[token_type]);
    return token;
}