#include "modules_list_node.h"
#include "utility.h"

static void print_node(const void *address, int depth);


ModulesListNode *create_modules_list_node() {
    ModulesListNode *modules_list_node = malloc_safe(sizeof(ModulesListNode));
    ast_number_of_nodes += 1;
    ast_allocated_memory += sizeof(ModulesListNode);
    modules_list_node->ast_node = (struct ASTNode) {
            .name = "MODULES LIST NODE",
            .tag = NODE_MODULE_LIST,
            .print_node = print_node,
    };
    modules_list_node->ast_node.tag = NODE_MODULE_LIST;
    modules_list_node->ast_node.name = "MODULES LIST NODE";
    modules_list_node->ast_node.print_node = print_node;
    // Todo change to Token*
    modules_list_node->modules_list = create_arraylist(sizeof(ModuleNode *));
    return modules_list_node;
}

static void print_node(const void *address, int depth) {
    const ModulesListNode *modules_list_node = address;
    puts("ModuleList\n");
    size_t size = get_arraylist_size(modules_list_node->modules_list);
    for (size_t i = 0; i < size; ++i) {
        ModuleNode *const *p_ast_node = get_element_at_index_arraylist(modules_list_node->modules_list, i);
        PRINT_AST_NODE(*p_ast_node, depth + 1);
    }
}
