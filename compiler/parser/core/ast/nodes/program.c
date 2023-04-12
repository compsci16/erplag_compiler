#include "core/ast/ast.h"
#include "modules_list_node.h"
#include "utility.h"
#include <stdio.h>

static void print_node(const void *address, int depth);

ProgramNode *create_program_node() {
    ProgramNode *program_node = malloc_safe(sizeof(ProgramNode));
    ast_number_of_nodes = 0;
    ast_allocated_memory = 0;
    ast_number_of_nodes += 1;
    ast_allocated_memory += sizeof(ProgramNode);
    program_node->ast_node = (ASTNode) {
            .name = "PROGRAM NODE",
            .tag = NODE_PROGRAM,
            .print_node = print_node
    };
    program_node->driver_module = NULL;
    program_node->module_declarations_node = NULL;
    program_node->modules_list_node = create_modules_list_node();
    return program_node;
}

static void print_node(const void *address, int depth) {
    const ProgramNode *program_node = address;
    printf("ProgramNode\n");
    PRINT_AST_NODE(program_node->driver_module, depth + 1);
    PRINT_AST_NODE(program_node->module_declarations_node, depth + 1);
    PRINT_AST_NODE(program_node->modules_list_node, depth + 1);
}


