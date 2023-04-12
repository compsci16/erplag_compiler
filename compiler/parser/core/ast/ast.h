#ifndef COMPILER_AST_H
#define COMPILER_AST_H

#include "datastructures/arraylist/arraylist.h"
#include "stack.h"
#include "ast_def.h"
#include "compiler/symbol_table/interface/symbol_table.h"
#include "lexer.h"

int ast_number_of_nodes;
int ast_allocated_memory;

typedef struct ModuleNode ModuleNode;
#define PRINT_AST_NODE(node, d) print_ast_node((const ASTNode *) (node), (d))
typedef enum {
    NODE_ARRAY_VARIABLE,
    NODE_BOOLEAN_LITERAL,
    NODE_CASE,
    NODE_GET_VALUE,
    NODE_INTEGER_LITERAL,
    NODE_MODULE_DECLARATIONS,
    NODE_MODULE,
    NODE_MODULE_LIST,
    NODE_PRINT_STATEMENT,
    NODE_PROGRAM,
    NODE_REAL_LITERAL,
    NODE_STATEMENTS,
    NODE_SWITCH,
    NODE_VARIABLE_LIST,
    NODE_VARIABLE,
    NODE_FOR,
    NODE_ARRAY,
    NODE_BINARY_OPERATOR,
    NODE_UNARY_OPERATOR,
    NODE_ID,
    NODE_DECLARE_VARIABLE_STATEMENT,
    NODE_DECLARE_ARRAY_VARIABLE_STATEMENT,
    NODE_ASSIGNMENT_STATEMENT,
    NODE_WHILE_STATEMENT,
    NODE_MODULE_USE_STATEMENT,
    FINAL_ENTRY_COUNT_OF_NODE_TYPES
} NODE_TYPE_TAG;


typedef struct ASTNode {
    NODE_TYPE_TAG tag;
    char *name;

    void (*print_node)(const void *, int);
} ASTNode;

typedef struct {
    ASTNode ast_node;
    SymbolTableEntry ***entries;
    ArrayList *statements_list; // list of StatementsNode*
} StatementsNode;


typedef struct {
    ASTNode ast_node;
    ArrayList *module_ids; // list of module IDs
} ModuleDeclarationsNode;

typedef struct {
    ASTNode ast_node;
    ArrayList *modules_list; // list of ModuleNode*
} ModulesListNode;

typedef struct VariableListNode {
    ASTNode ast_node;
    ArrayList *variable_nodes_list; // list of VariableNode* or ArrayVariableNode*
} VariableListNode;

typedef struct {
    ASTNode ast_node;
    Token *id_token;
    SymbolTable *symbol_table;
    // populate during ST construction
    TypeDescriptor identifier_type_descriptor;
    int offset;
} IDNode;


typedef struct {
    ASTNode ast_node;
    IDNode *id;
    const Token *type;
    SymbolTableEntry *symbol_table_entry;
} VariableNode;


typedef struct {
    ASTNode ast_node;
    ModuleDeclarationsNode *module_declarations_node;
    ModuleNode *driver_module;
    ModulesListNode *modules_list_node;
    SymbolTable *global_symbol_table;
    SymbolTable *module_declarations_symbol_table;
} ProgramNode;

typedef struct {
    ASTNode ast_node;
    ArrayList *variable_nodes; // list of VariableNode*
} DeclareVariableStatementNode;

typedef struct {
    ASTNode ast_node;
    ArrayList *array_variable_nodes; // list of ArrayVariableNode*
} DeclareArrayVariableStatementNode;


typedef struct {
    ASTNode ast_node;
    VariableNode *variable_node;
    StatementsNode *statements_node;
    Range for_range;
    int start_line;
    int end_line;
    SymbolTable *for_symbol_table;
} ForNode;

typedef struct {
    ASTNode ast_node;
    IDNode *id;
    const Token *type;
    Range array_range;
    SymbolTableEntry *symbol_table_entry;

} ArrayVariableNode;

typedef struct {
    ASTNode ast_node;
    IDNode *id;
} GetValNode;

typedef struct {
    ASTNode ast_node;
    const Token *num;
    TypeDescriptor integer_literal_type_descriptor;
    int result_offset;
} IntegerLiteralNode;

typedef struct {
    ASTNode ast_node;
    const Token *r_num;
    TypeDescriptor real_literal_type_descriptor;
} RealLiteralNode;

typedef struct {
    ASTNode ast_node;
    TypeDescriptor boolean_literal_type_descriptor;
    const Token *boolean_const;
    int result_offset;
} BooleanLiteralNode;

typedef struct {
    ASTNode ast_node;
    TypeDescriptor binary_operator_type_descriptor;
    Token *operator;
    ASTNode *left;
    ASTNode *right;
    int result_offset;
} BinaryOperatorNode;

typedef struct {
    ASTNode ast_node;
    Token *operator;
    enum Tag {
        UNARY_FACTOR,
        UNARY_EXPRESSION
    } unary_tag;
    TypeDescriptor unary_operator_type_descriptor;
    ASTNode *operand;
    int result_offset;
} UnaryOperatorNode;

typedef struct ModuleNode {
    ASTNode ast_node;
    IDNode *module_id;
    VariableListNode *input_parameter_list_node;
    VariableListNode *output_parameter_list_node;
    StatementsNode *statements_node;
    int start_line;
    int end_line;
    SymbolTableEntry *symbol_table_entry;
    SymbolTable *function_entry_symbol_table;
    SymbolTable *function_scope_symbol_table;
} ModuleNode;

typedef struct ArrayNode {
    ASTNode ast_node;
    TypeDescriptor array_type_descriptor;
    IDNode *array_id;
    ASTNode *array_expression; // can be: Operator Node, Factor ID Node, Integer Literal Node, Boolean Literal Node
    int offset;
} ArrayNode;

typedef struct {
    ASTNode ast_node;
    enum PrintItemTag {
        REAL_LITERAL,
        INTEGER_LITERAL,
        BOOLEAN_LITERAL,
    } tag;
    ASTNode *print_item;
} PrintStatementNode;

typedef enum {
    BOOLEAN_CONSTANT,
    INTEGER_CONSTANT
} CaseTag;

typedef struct {
    ASTNode ast_node;
    CaseTag tag;
    Token *case_value;
    int start_line;
    int end_line;
    StatementsNode *statements_node;
    SymbolTable *case_symbol_table;
    TypeDescriptor type_descriptor;
} CaseStatementNode;


typedef struct {
    ASTNode ast_node;
    IDNode *id;
    ArrayList *case_list; // list of CaseStatementNode*
    int end_line;
    CaseStatementNode *default_case_statement_node;
} SwitchStatementNode;

typedef struct {
    ASTNode ast_node;
    ASTNode *lhs_node;
    ASTNode *rhs_node;
    int assign_line;
} AssignmentStatementNode;

typedef struct {
    ASTNode ast_node;
    ASTNode *predicate;
    int start_line;
    int end_line;
    StatementsNode *statements_node;
    SymbolTable *while_symbol_table;
} WhileStatementNode;

typedef struct {
    ASTNode ast_node;
    IDNode *module_name;
    bool has_assign_op_tag;
    // List of IDNode*
    ArrayList *result_id_list;
    // List of ASTNode* with tag ID or ArrayNode
    ArrayList *parameter_list;
    SymbolTable *parent_symbol_table;
    bool is_recursive;
    TypeDescriptor caller_type_descriptor;
} ModuleUseStatementNode;

static inline void print_ast_node(const ASTNode *node, int depth) {
    for (int i = 0; i < depth; ++i) {
        printf(" | ");
    }
    if (node->print_node != NULL) {
        node->print_node(node, depth);
    } else {
        puts("Undefined print_node");
    }
}

void execute_action_on_semantic_stack(Stack *semantic_stack, int action, Lexer *lexer);

#endif //COMPILER_AST_H
