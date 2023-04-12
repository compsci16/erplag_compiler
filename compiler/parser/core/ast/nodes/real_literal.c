#include "real_literal.h"
#include "utility.h"
#include "core/ast/ast.h"

static void print_node(const void *address, int depth);


RealLiteralNode *create_real_literal_node() {
    RealLiteralNode *real_literal_node = malloc_safe(sizeof(RealLiteralNode));
    ast_number_of_nodes += 1;
    ast_allocated_memory += sizeof(RealLiteralNode);
    real_literal_node->ast_node = (struct ASTNode) {
            .name = "REAL LITERAL NODE",
            .tag = NODE_REAL_LITERAL,
            .print_node = print_node,
    };
    real_literal_node->r_num = NULL;
    return real_literal_node;
}

static void print_node(const void *address, int depth) {
    const RealLiteralNode *real_literal_node = address;
    printf("RealLiteralNode = %s\n", real_literal_node->r_num->lexeme);
}

