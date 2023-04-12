#ifndef COMPILER_PARSE_TREEX_H
#define COMPILER_PARSE_TREEX_H

#include "datastructures/arraylist/arraylist.h"
#include "datastructures/stack/stack.h"
#include "ast.h"

#define ROOT_PARENT -1

int parse_tree_number_of_nodes;
int parse_tree_allocated_memory;

typedef struct Parser Parser;
typedef struct TreeNode {
    int symbol;
    // array of pointers to children
    ArrayList *children;
    Token *token;
    int parent_symbol;
    bool popped_from_stack;
} TreeNode;

typedef TreeNode *TreeNodePtr;

typedef struct ParseTree {
    TreeNode *root;
    ProgramNode *program_node;
    Stack *semantic_stack;
    bool got_syntax_error;
} ParseTree;

ParseTree *construct_parse_tree(Parser *parser);

void print_parse_tree(Parser *parser, struct ParseTree *parse_tree, FILE *fp);

#endif //COMPILER_PARSE_TREEX_H
