#include "module_node.h"
#include "statements.h"
#include "variable_list.h"
#include "utility.h"

static void print_node(const void *address, int d);


ModuleNode *create_module_node() {
    ModuleNode *module_node = malloc_safe(sizeof(ModuleNode));
    ast_number_of_nodes += 1;
    ast_allocated_memory += sizeof(ModuleNode);
    module_node->ast_node = (struct ASTNode) {
            .name = "MODULE NODE",
            .tag = NODE_MODULE,
            .print_node = print_node,
    };
    module_node->module_id = NULL;
    module_node->statements_node = create_statements_node();
    module_node->input_parameter_list_node = NULL;
    module_node->output_parameter_list_node = NULL;
    return module_node;
}

static void print_node(const void *address, int d) {
    const ModuleNode *module_node = address;
    printf("ModuleNode\n");
    PRINT_AST_NODE(module_node->module_id, d + 1);
    // printf("Name = %s\n", module_node->module_id->lexeme);
    for (int i = 0; i < d; ++i) {
        printf(" | ");
    }
    puts("ModuleNode IPL:");
    if (module_node->input_parameter_list_node != NULL)
        PRINT_AST_NODE(module_node->input_parameter_list_node, d + 1);
    for (int i = 0; i < d; ++i) {
        printf(" | ");
    }
    puts("ModuleNode OPL ");
    if (module_node->output_parameter_list_node != NULL)
        PRINT_AST_NODE(module_node->output_parameter_list_node, d + 1);
    for (int i = 0; i < d; ++i) {
        printf(" | ");
    }
    puts("Module Statements");
    if (module_node->statements_node != NULL)
        PRINT_AST_NODE(module_node->statements_node, d + 1);
}
