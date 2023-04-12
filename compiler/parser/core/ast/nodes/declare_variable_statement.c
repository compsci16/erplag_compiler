#include "declare_variable_statement.h"
#include "utility.h"

static void print_node(const void *address, int depth);


DeclareVariableStatementNode *create_declare_variable_statement_node() {
    DeclareVariableStatementNode *ast_node = malloc_safe(sizeof(DeclareVariableStatementNode));
    ast_number_of_nodes += 1;
    ast_allocated_memory += sizeof(DeclareVariableStatementNode);
    ast_node->ast_node = (struct ASTNode) {
            .name = "DECLARE VARIABLE STATEMENTS NODE",
            .tag = NODE_DECLARE_VARIABLE_STATEMENT,
            .print_node = print_node,
    };
    ast_node->variable_nodes = create_arraylist(sizeof(VariableNode *));
    return ast_node;
}

static void print_node(const void *address, int depth) {
    printf("DeclareVariableStatementNode\n");
    const DeclareVariableStatementNode *declare_variable_statement_node = address;
    ArrayList *variable_nodes = declare_variable_statement_node->variable_nodes;
    for (size_t i = 0; i < variable_nodes->size; ++i) {
        VariableNode **p_variable_node = get_element_at_index_arraylist(variable_nodes, i);
        PRINT_AST_NODE(*p_variable_node, depth + 1);
    }
}
