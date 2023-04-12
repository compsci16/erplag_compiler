#include "core/ast/ast.h"
#include "statements.h"
#include "utility.h"

static void print_node(const void *data, int d);

CaseStatementNode *create_case_node() {
    CaseStatementNode *case_node = malloc_safe(sizeof(CaseStatementNode));
    ast_number_of_nodes += 1;
    ast_allocated_memory += sizeof(CaseStatementNode);
    case_node->ast_node = (struct ASTNode) {
            .name = "CASE NODE",
            .tag = NODE_CASE,
            .print_node = print_node,
    };
    return case_node;
}

static void print_node(const void *data, int d) {
    const CaseStatementNode *case_node = data;
    return;
    printf("CASE Node: Case Tag %d, Case Value: %s\n", case_node->tag, case_node->case_value->lexeme);
    PRINT_AST_NODE(case_node->statements_node, d + 1);
}
