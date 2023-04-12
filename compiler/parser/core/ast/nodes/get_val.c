#include "core/ast/ast.h"
#include "utility.h"
#include <stdio.h>


static void print_node(const void *address, int depth);


GetValNode *create_get_val_statement_node() {
    GetValNode *get_val_node = malloc_safe(sizeof(GetValNode));
    ast_number_of_nodes += 1;
    ast_allocated_memory += sizeof(GetValNode);
    get_val_node->ast_node = (ASTNode) {
            .name = "GETVAL NODE",
            .tag = NODE_GET_VALUE,
            .print_node=print_node,
    };
    get_val_node->id = NULL;
    return get_val_node;
}

static void print_node(const void *address, int depth) {
    const GetValNode *gv_data = address;
    printf("GetValueNode\n");
    PRINT_AST_NODE(gv_data->id, depth + 1);
}
