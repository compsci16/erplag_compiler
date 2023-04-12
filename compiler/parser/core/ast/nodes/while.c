#include "while.h"
#include "utility.h"

static void print_node(const void *address, int depth);

WhileStatementNode *create_while_statement_node() {
    WhileStatementNode *ast_node = malloc_safe(sizeof(WhileStatementNode));
    ast_number_of_nodes += 1;
    ast_allocated_memory += sizeof(WhileStatementNode);
    ast_node->ast_node = (ASTNode) {
            .name = "WHILE STATEMENT NODE",
            .tag = NODE_WHILE_STATEMENT,
            .print_node = print_node,

    };
    ast_node->statements_node = NULL;
    ast_node->predicate = NULL;
    return ast_node;
}

static void print_node(const void *address, int depth) {
    printf("WhileStatementNode\n");
    const WhileStatementNode *while_statement_node = address;
    PRINT_AST_NODE(while_statement_node->statements_node, depth + 1);
}


