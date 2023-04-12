#include "variable_list.h"
#include "core/ast/ast.h"
#include "utility.h"

static void print_node(const void *address, int depth);

VariableListNode *create_variable_list_node() {
    VariableListNode *variable_list_node = malloc_safe(sizeof(VariableListNode));
    ast_number_of_nodes += 1;
    ast_allocated_memory += sizeof(VariableListNode);
    variable_list_node->ast_node = (ASTNode) {
            .name = "VARIABLE LIST NODE",
            .tag = NODE_VARIABLE_LIST,
            .print_node = print_node,
    };
    // list of variables or array variables
    variable_list_node->variable_nodes_list = create_arraylist(sizeof(ASTNode *));
    return variable_list_node;
}

static void print_node(const void *address, int depth) {
    const VariableListNode *variable_list_node = address;
    printf("VariableListNode\n");
    ArrayList *variables_list = variable_list_node->variable_nodes_list;
    size_t size = get_arraylist_size(variables_list);
    for (size_t i = 0; i < size; ++i) {
        const ASTNode *const *node = get_element_at_index_arraylist(variables_list, i);
        PRINT_AST_NODE(*node, depth + 1);
    }
}


