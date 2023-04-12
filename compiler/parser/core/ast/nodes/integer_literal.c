#include "integer_literal.h"
#include "utility.h"
#include "core/ast/ast.h"

static void print_node(const void *address, int d);

IntegerLiteralNode *create_integer_literal_node() {
    IntegerLiteralNode *integer_literal_node = malloc_safe(sizeof(IntegerLiteralNode));
    ast_number_of_nodes += 1;
    ast_allocated_memory += sizeof(IntegerLiteralNode);
    integer_literal_node->ast_node = (struct ASTNode) {
            .name = "INTEGER LITERAL NODE",
            .tag = NODE_INTEGER_LITERAL,
            .print_node = print_node,
    };
    integer_literal_node->ast_node.tag = NODE_INTEGER_LITERAL;
    integer_literal_node->ast_node.name = "INTEGER LITERAL NODE";
    integer_literal_node->ast_node.print_node = print_node;
    integer_literal_node->num = NULL;
    integer_literal_node->integer_literal_type_descriptor = (TypeDescriptor) {.form = TYPE_INTEGER};
    return integer_literal_node;
}

static void print_node(const void *address, int d) {
    const IntegerLiteralNode *integer_literal_node = address;
    printf("IntegerLiteralNode for = %s\n", integer_literal_node->num->lexeme);
}
