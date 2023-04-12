#include "unary_operator.h"
#include "utility.h"

static void print_node(const void *address, int depth);


UnaryOperatorNode *create_unary_operator_node() {
    UnaryOperatorNode *unary_operator_node = malloc_safe(sizeof(UnaryOperatorNode));
    ast_number_of_nodes += 1;
    ast_allocated_memory += sizeof(UnaryOperatorNode);
    unary_operator_node->ast_node = (struct ASTNode) {
            .name = "UNARY OPERATOR NODE",
            .tag = NODE_UNARY_OPERATOR,
            .print_node = print_node,
    };
    unary_operator_node->operator = NULL;
    unary_operator_node->operand = NULL;
    return unary_operator_node;
}

static void print_node(const void *address, int depth) {
    const UnaryOperatorNode *unary_operator_node = address;
    printf("UnaryOperatorNode : %s\n", unary_operator_node->operator->lexeme);
    PRINT_AST_NODE(unary_operator_node->operand, depth + 1);
}

