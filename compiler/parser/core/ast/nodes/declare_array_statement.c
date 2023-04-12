#include "declare_array_statement.h"
#include "utility.h"

static void print_node(const void *address, int d);


DeclareArrayVariableStatementNode *create_declare_array_statement_node() {
    DeclareArrayVariableStatementNode *declare_array_variable_statement_node =
            malloc_safe(sizeof(DeclareArrayVariableStatementNode));
    ast_number_of_nodes += 1;
    ast_allocated_memory += sizeof(DeclareArrayVariableStatementNode);
    declare_array_variable_statement_node->ast_node = (struct ASTNode) {
            .name = "DECLARE ARRAY VARIABLE STATEMENT",
            .tag = NODE_DECLARE_ARRAY_VARIABLE_STATEMENT,
            .print_node = print_node,
    };
    declare_array_variable_statement_node->ast_node.tag = NODE_DECLARE_ARRAY_VARIABLE_STATEMENT;
    declare_array_variable_statement_node->ast_node.name = "DECLARE ARRAY VARIABLE STATEMENT";
    declare_array_variable_statement_node->ast_node.print_node = print_node;
    declare_array_variable_statement_node->array_variable_nodes = create_arraylist(sizeof(ArrayVariableNode *));
    return declare_array_variable_statement_node;
}

static void print_node(const void *address, int depth) {
    printf("DeclareArrayVariableStatementNode");
    const DeclareArrayVariableStatementNode *declare_array_variable_statement_node = address;
    ArrayList *array_variable_nodes = declare_array_variable_statement_node->array_variable_nodes;
    for (size_t i = 0; i < array_variable_nodes->size; ++i) {
        ArrayVariableNode **ar_var_node = get_element_at_index_arraylist(array_variable_nodes, i);
        PRINT_AST_NODE(*ar_var_node, depth + 1);
    }
}
