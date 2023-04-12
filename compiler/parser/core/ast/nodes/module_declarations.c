#include "module_declarations.h"
#include "utility.h"

static void print_node(const void *address, int d);

ModuleDeclarationsNode *create_module_declarations_node() {
    ModuleDeclarationsNode *module_declarations_node = malloc_safe(sizeof(ModuleDeclarationsNode));
    ast_number_of_nodes += 1;
    ast_allocated_memory += sizeof(ModuleDeclarationsNode);
    module_declarations_node->ast_node = (struct ASTNode) {
            .name = "MODULE DECLARATIONS NODE",
            .tag = NODE_MODULE_DECLARATIONS,
            .print_node = print_node
    };
    module_declarations_node->module_ids = create_arraylist(sizeof(IDNode *));
    return module_declarations_node;
}


static void print_node(const void *address, int d) {
    const ModuleDeclarationsNode *module_declarations_node = address;
    printf("ModuleDeclarationsNode for:  \n");
    size_t size = get_arraylist_size(module_declarations_node->module_ids);
    for (size_t i = 0; i < size; ++i) {
        const char **mod_id = get_element_at_index_arraylist(module_declarations_node->module_ids, i);
        PRINT_AST_NODE(*mod_id, d + 1);
    }
}
