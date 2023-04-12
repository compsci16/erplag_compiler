#include "module_use_statement.h"
#include "utility.h"

static void print_node(const void *address, int depth);


ModuleUseStatementNode *create_module_use_statement_node() {
    ModuleUseStatementNode *module_use_statement_node = malloc_safe(sizeof(ModuleUseStatementNode));
    ast_number_of_nodes += 1;
    ast_allocated_memory += sizeof(ModuleUseStatementNode);
    module_use_statement_node->ast_node = (struct ASTNode) {
            .name = "MODULE USE STATEMENT NODE",
            .tag = NODE_MODULE_USE_STATEMENT,
            .print_node = print_node
    };
    module_use_statement_node->result_id_list = create_arraylist(sizeof(IDNode *));
    module_use_statement_node->parameter_list = create_arraylist(sizeof(ASTNode *));
    module_use_statement_node->has_assign_op_tag = false;
    module_use_statement_node->parent_symbol_table = NULL;
    return module_use_statement_node;
}

static void print_node(const void *address, int depth) {
    const ModuleUseStatementNode *mod_use_stmt_node = address;
    printf("ModuleUseStmtNode with has_assign_op = %d\n", mod_use_stmt_node->has_assign_op_tag);
    PRINT_AST_NODE(mod_use_stmt_node->module_name, depth + 1);
    ArrayList *id_list = mod_use_stmt_node->result_id_list;
    for (int i = 0; i < id_list->size; ++i) {
        IDNode **id_node = get_element_at_index_arraylist(id_list, i);
        PRINT_AST_NODE(*id_node, depth + 1);
    }
    ArrayList *parameter_list = mod_use_stmt_node->parameter_list;
    for (int i = 0; i < parameter_list->size; ++i) {
        ASTNode **para_node = get_element_at_index_arraylist(parameter_list, i);
        PRINT_AST_NODE(*para_node, depth + 1);
    }
}
