#include "node_utility.h"

const char *get_identifier_from(ASTNode *ast_node) {
    if (ast_node->tag == NODE_ARRAY) {
        return ((ArrayNode *) ast_node)->array_id->id_token->lexeme;
    } else if (ast_node->tag == NODE_ID) {
        return ((IDNode *) ast_node)->id_token->lexeme;
    }
    return NULL;
}

int get_boolean_value_from(BooleanLiteralNode *boolean_literal_node) {
    if (boolean_literal_node->boolean_const->token_type == TRUE) {
        return 1;
    } else {
        return 0;
    }
}

RangeInInt get_range_in_int_from(Range range) {
    int begin_range = atoi(range.begin->lexeme);
    int end_range = atoi(range.end->lexeme);
    if (range.begin_sign->token_type == MINUS) {
        begin_range *= -1;
    }
    if (range.end_sign->token_type == MINUS) {
        end_range *= -1;
    }
    int size = end_range - begin_range + 1;
    return (RangeInInt) {.begin = begin_range, .end = end_range, .size = size};
}

TypeDescriptor get_type_descriptor(ASTNode *node) {
    switch (node->tag) {
        case NODE_ID:
            return ((IDNode *) node)->identifier_type_descriptor;
        case NODE_ARRAY:
            return ((ArrayNode *) node)->array_type_descriptor;
        case NODE_UNARY_OPERATOR:
            return ((UnaryOperatorNode *) node)->unary_operator_type_descriptor;
        case NODE_BOOLEAN_LITERAL:
            return ((BooleanLiteralNode *) node)->boolean_literal_type_descriptor;
        case NODE_INTEGER_LITERAL:
            return ((IntegerLiteralNode *) node)->integer_literal_type_descriptor;
        case NODE_REAL_LITERAL:
            return ((RealLiteralNode *) node)->real_literal_type_descriptor;
        case NODE_BINARY_OPERATOR:
            return ((BinaryOperatorNode *) node)->binary_operator_type_descriptor;
            // TODO append with others;
        default:
            return (TypeDescriptor) {.form = TYPE_UNDEFINED};
    }
}