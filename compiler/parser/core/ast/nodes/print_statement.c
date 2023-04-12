#include "print_statement.h"
#include "utility.h"

static void print_node(const void *address, int depth);

PrintStatementNode *create_print_statement_node() {
    PrintStatementNode *print_statement_node = malloc_safe(sizeof(PrintStatementNode));
    ast_number_of_nodes += 1;
    ast_allocated_memory += sizeof(PrintStatementNode);
    print_statement_node->ast_node = (struct ASTNode) {
            .tag = NODE_PRINT_STATEMENT,
            .name = "PRINT STATEMENT NODE",
            .print_node = print_node,
    };
    return print_statement_node;
}

static void print_node(const void *address, int depth) {
    printf("PrintStatementNode\n");
    const PrintStatementNode *print_statement_node = address;
    PRINT_AST_NODE(print_statement_node->print_item, depth + 1);
}
