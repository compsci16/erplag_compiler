#include "array.h"
#include "utility.h"


static void print_node(const void *ast_node, int indentation_level);

static void accept_array_node(ArrayNode *array_node, Visitor *visitor);

ArrayNode *create_array_node() {
    ArrayNode *array_node = malloc_safe(sizeof(struct ArrayNode));
    ast_number_of_nodes += 1;
    ast_allocated_memory += sizeof(struct ArrayNode);
    array_node->ast_node = (ASTNode) {
            .name = "Array NODE",
            .tag = NODE_ARRAY,
            .print_node=print_node,
    };
    array_node->array_id = NULL;
    array_node->array_expression = NULL;
    array_node->offset = 0;
    return array_node;
}


static void print_node(const void *ast_node, int depth) {
    ArrayNode *array_node = (ArrayNode *) ast_node;
    printf("Array Node:\n");
    PRINT_AST_NODE(array_node->array_id, depth + 1);
    PRINT_AST_NODE(array_node->array_expression, depth + 1);
}
