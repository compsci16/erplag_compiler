#include "variable_node.h"
#include "utility.h"

static void print_node(const void *address, int depth);


VariableNode *create_variable_node() {
    VariableNode *variable_node = malloc_safe(sizeof(VariableNode));
    ast_number_of_nodes += 1;
    ast_allocated_memory += sizeof(VariableNode);
    variable_node->ast_node = (struct ASTNode) {
            .name = "VARIABLE NODE",
            .tag = NODE_VARIABLE,
            .print_node = print_node,
    };
    return variable_node;
}

static void print_node(const void *address, int depth) {
    const VariableNode *variable_node = address;
    printf("VariableNode Type := %s\n", variable_node->type->lexeme);
    PRINT_AST_NODE(variable_node->id, depth + 1);
    //  printf("Stored: Name = %s Type = %s\n", variable_node->id->lexeme, variable_node->type->lexeme);
}



