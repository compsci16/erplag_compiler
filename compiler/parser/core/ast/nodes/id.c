#include "id.h"
#include "utility.h"

static void print_node(const void *address, int depth);


IDNode *create_id_node() {
    IDNode *id_node = malloc_safe(sizeof(IDNode));
    ast_number_of_nodes += 1;
    ast_allocated_memory += sizeof(IDNode);
    id_node->ast_node = (struct ASTNode) {
            .tag = NODE_ID,
            .name = "FACTOR ID NODE",
            .print_node= print_node
    };
    return id_node;
}

IDNode *create_id_node_with_token(Token *token) {
    IDNode *id_node = create_id_node();
    id_node->id_token = token;
    return id_node;
}

static void print_node(const void *address, int depth) {
    const IDNode *id_node = address;
    printf("IDNode for %s\n", id_node->id_token->lexeme);
}
