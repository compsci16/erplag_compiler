#ifndef COMPILER_NODE_UTILITY_H
#define COMPILER_NODE_UTILITY_H

#include "ast.h"

// Returns identifier from ArrayNode or IDNode
#define GET_IDENTIFIER_FROM(ast_node) \
    ((ast_node)->tag == NODE_ARRAY ? \
        ((ArrayNode *) (ast_node))->array_id->id_token->lexeme : \
        ((ast_node)->tag == NODE_ID ? \
            ((IDNode *) (ast_node))->id_token->lexeme : NULL))

TypeDescriptor get_type_descriptor(ASTNode *node);

int get_boolean_value_from(BooleanLiteralNode *boolean_literal_node);

typedef struct RangeInInt {
    int begin;
    int end;
    int size;
} RangeInInt;

RangeInInt get_range_in_int_from(Range range);

#endif //COMPILER_NODE_UTILITY_H
