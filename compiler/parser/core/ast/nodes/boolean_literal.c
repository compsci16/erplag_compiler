#include "boolean_literal.h"
#include "utility.h"
#include "core/ast/ast.h"


static void print_node(const void *data, int d);

static void accept_boolean_literal_node(BooleanLiteralNode *boolean_literal_node, Visitor *visitor);

BooleanLiteralNode *create_boolean_literal_node() {
    BooleanLiteralNode *boolean_literal_node = malloc_safe(sizeof(BooleanLiteralNode));
    ast_number_of_nodes += 1;
    ast_allocated_memory += sizeof(BooleanLiteralNode);
    boolean_literal_node->ast_node = (struct ASTNode) {
            .name = "BOOLEAN LITERAL NODE",
            .tag = NODE_BOOLEAN_LITERAL,
            .print_node = print_node,
    };
    boolean_literal_node->boolean_const = NULL;
    boolean_literal_node->boolean_literal_type_descriptor = (TypeDescriptor) {.form = TYPE_BOOLEAN};
    return boolean_literal_node;
}

static void print_node(const void *data, int d) {
    const BooleanLiteralNode *boolean_literal_node = data;
    printf("Boolean Literal = %s\n", boolean_literal_node->boolean_const->lexeme);
}
