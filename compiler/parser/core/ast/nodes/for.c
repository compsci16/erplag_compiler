#include "for.h"
#include "core/ast/ast.h"
#include "utility.h"

static void print_node(const void *data, int d);

ForNode *create_for_node() {
    ForNode *for_node = malloc_safe(sizeof(ForNode));
    ast_number_of_nodes += 1;
    ast_allocated_memory += sizeof(ForNode);
    for_node->ast_node = (struct ASTNode) {
            .name = "FOR NODE",
            .tag = NODE_FOR,
            .print_node = print_node,
    };
    for_node->statements_node = NULL;
    for_node->variable_node = NULL;
    return for_node;
}

static void print_node(const void *data, int d) {
    const ForNode *for_node = data;
    printf("ForNode : ");
    printf("For Node Range: ");
    printf("%s %s to %s %s ",
           for_node->for_range.begin_sign->lexeme,
           for_node->for_range.begin->lexeme,
           for_node->for_range.end_sign->lexeme,
           for_node->for_range.end->lexeme);
    printf("For Node counter: \n");
    PRINT_AST_NODE(for_node->variable_node, d + 1);
    PRINT_AST_NODE(for_node->statements_node, d + 1);
}
