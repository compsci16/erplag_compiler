#include "core/ast/ast.h"
#include "utility.h"
#include <stdio.h>

static void print_node(const void *address, int depth);


StatementsNode *create_statements_node() {
    StatementsNode *statements_node = malloc_safe(sizeof(StatementsNode));
    ast_number_of_nodes += 1;
    ast_allocated_memory += sizeof(StatementsNode);
    statements_node->ast_node = (ASTNode) {
            .name = "STATEMENTS NODE",
            .tag = NODE_STATEMENTS,
            .print_node = print_node,
    };
    statements_node->statements_list = create_arraylist(sizeof(ASTNode *));
    return statements_node;
}

static void print_node(const void *address, int depth) {
    const StatementsNode *statements_node = address;
    printf("StatementsNode\n");
    for (int i = 0; i < get_arraylist_size(statements_node->statements_list); ++i) {
        const ASTNode **p_ast_node = (const ASTNode **)
                get_element_at_index_arraylist(statements_node->statements_list, i);
        PRINT_AST_NODE(*p_ast_node, depth + 1);
    }
}


