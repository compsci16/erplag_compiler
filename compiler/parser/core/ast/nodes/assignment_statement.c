#include "assignment_statement.h"
#include "utility.h"

static void print_node(const void *address, int depth);

AssignmentStatementNode *create_assignment_statement_node() {
    AssignmentStatementNode *ast_node = malloc_safe(sizeof(AssignmentStatementNode));
    ast_number_of_nodes += 1;
    ast_allocated_memory += sizeof(AssignmentStatementNode);
    ast_node->ast_node = (ASTNode) {
            .name = "Assignment Statement Node",
            .tag = NODE_ASSIGNMENT_STATEMENT,
            .print_node=print_node,
    };
    ast_node->rhs_node = NULL;
    ast_node->lhs_node = NULL;
    return ast_node;
}


static void print_node(const void *address, int depth) {
    const AssignmentStatementNode *assignment_statement_node = address;
    printf("AssignmentNode: \n");
    PRINT_AST_NODE(assignment_statement_node->lhs_node, depth + 1);
    PRINT_AST_NODE(assignment_statement_node->rhs_node, depth + 1);
}
