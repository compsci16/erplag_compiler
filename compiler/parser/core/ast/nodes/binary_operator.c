#include "binary_operator.h"
#include "utility.h"

static void print_node(const void *address, int d);

static void accept_binary_operator_node(BinaryOperatorNode *binary_operator_node, Visitor *visitor);

BinaryOperatorNode *create_operator_node() {
    BinaryOperatorNode *op_node = malloc_safe(sizeof(BinaryOperatorNode));
    ast_number_of_nodes += 1;
    ast_allocated_memory += sizeof(BinaryOperatorNode);
    op_node->ast_node = (struct ASTNode) {
            .name = "OPERATOR NODE",
            .tag = NODE_BINARY_OPERATOR,
            .print_node = print_node,
    };
    op_node->left = NULL;
    op_node->right = NULL;
    return op_node;
}


static void print_node(const void *address, int d) {
    const BinaryOperatorNode *operator_node = address;
    printf("Operator Node for %s\n", operator_node->operator->lexeme);
    PRINT_AST_NODE(operator_node->left, d + 1);
    PRINT_AST_NODE(operator_node->right, d + 1);
}
