#include "array_variable.h"
#include "utility.h"

static void print_node(const void *address, int depth);

static void accept_array_variable_node(ArrayVariableNode *array_variable_node, Visitor *visitor);

ArrayVariableNode *create_array_variable_node() {
    ArrayVariableNode *array_variable_node = malloc_safe(sizeof(ArrayVariableNode));
    ast_number_of_nodes += 1;
    ast_allocated_memory += sizeof(ArrayVariableNode);
    array_variable_node->ast_node = (struct ASTNode) {
            .name = "ARRAY VARIABLE NODE",
            .tag = NODE_ARRAY_VARIABLE,
            .print_node = print_node,
    };
    return array_variable_node;
}

static void print_node(const void *address, int depth) {
    const ArrayVariableNode *avd = address;
    printf("ArrayVariableNode type = %s; range = [%s%s, %s%s]\n", avd->type->lexeme,
           avd->array_range.begin_sign->lexeme,
           avd->array_range.begin->lexeme,
           avd->array_range.end_sign->lexeme,
           avd->array_range.end->lexeme);
    PRINT_AST_NODE(avd->id, depth + 1);
}
