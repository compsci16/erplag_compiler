#include "switch.h"
#include "utility.h"
#include "statements.h"

static void print_node(const void *address, int depth);


SwitchStatementNode *create_switch_node() {
    SwitchStatementNode *switch_node = malloc_safe(sizeof(SwitchStatementNode));
    ast_number_of_nodes += 1;
    ast_allocated_memory += sizeof(SwitchStatementNode);
    switch_node->ast_node = (struct ASTNode) {
            .tag = NODE_SWITCH,
            .name = "SWITCH NODE",
            .print_node = print_node,
    };
    switch_node->case_list = create_arraylist(sizeof(ASTNode *));
    switch_node->default_case_statement_node = NULL;
    return switch_node;
}

static void print_node(const void *address, int depth) {
    const SwitchStatementNode *switch_node = address;
    //  printf("SWITCH ID %s", switch_node->id); //id changed to node;
    size_t case_list_size = get_arraylist_size(switch_node->case_list);
    printf("SwitchNode\n");
    for (int i = 0; i < case_list_size; ++i) {
        const ASTNode **case_node = (const ASTNode **)
                get_element_at_index_arraylist(switch_node->case_list, i);
        PRINT_AST_NODE(*case_node, depth + 1);
    }
    if (switch_node->default_case_statement_node != NULL)
        PRINT_AST_NODE(switch_node->default_case_statement_node, depth + 1);
}



