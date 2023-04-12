#ifndef COMPILER_TOKEN_H
#define COMPILER_TOKEN_H


typedef struct Token {
    int token_type;
    char *lexeme;
    int line_number;
} Token;

Token *create_token_with_type_and_lexeme_for(int token_type);

#endif //COMPILER_TOKEN_H
