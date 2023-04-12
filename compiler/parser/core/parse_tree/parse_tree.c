#include <ctype.h>
#include "utility/utility.h"
#include "datastructures/stack/stack.h"
#include "utility/filehandler.h"
#include "utility/int.h"
#include "utility/string_def.h"
#include "grammar/grammar_populator/grammar_constants.h"
#include "compiler/symbol_table/constructor/symbol_table_constructor.h"
#include "token.h"
#include "parse_tree.h"
#include "lexer.h"
#include "grammar/cfg.h"
#include "interface/parser.h"
#include "compiler/error_reporter/error_report.h"
#include "compiler/semantic_analyzer/semantic_analyzer.h"
#include "compiler/code_generator/code_generator.h"

const char *PARSER_LOG_FILE = "./parser_log.txt";
FILE *parser_log;
int log_num;

static void destroy_parse_tree(ParseTree *parse_tree);

static void destroy_parse_tree_node(TreeNode *node);

static void print_stack_to_log(Stack *parser_stack);

static void format_and_print_leaf_non_terminal(TreeNodePtr node, FILE *fp);

static Token *create_epsilon_token(int line_number);

static TreeNode *create_empty_tree_node_for(int symbol);

static void format_and_print_non_leaf_non_terminal(TreeNodePtr node, FILE *fp);

static void format_and_print_leaf_terminal(TreeNodePtr tree_node_ptr, FILE *fp);


static void print_tree_node(Parser *parser, TreeNodePtr node, FILE *fp);

static Token *
handle_terminal_on_top(Stack *stack, Lexer *lexer, TreeNodePtr const *top_of_stack, Token *token,
                       int *last_syntax_error_line);

static Token *
handle_non_terminal_on_top(Stack *stack, const Parser *parser, Lexer *lexer, const Grammar *grammar,
                           TreeNodePtr const *top_of_stack, Token *token, HashSet *common_syn_set,
                           int *last_syntax_error_line, Stack *semantic_stack);

static HashSet *get_common_syn_set(const Grammar *grammar);


static HashSet *get_common_syn_set(const Grammar *grammar) {
    HashSet *common_syn_set = create_hashset(sizeof(int), hash_int, comparator_int);
    for (int i = 0; i < N_KEYWORDS; ++i) {
        char *token_for_keyword = get_upper(KEYWORDS[i]);
        int *token_type = get_from_hashmap(grammar->string_to_terminal_map, &token_for_keyword);
        add_to_hashset(common_syn_set, token_type);
        free(token_for_keyword);
    }
//    puts("Common Syn Set");
//    // TODO: REMOVE
//    print_all_keys_hashset(common_syn_set, print_int);
    return common_syn_set;
}

ParseTree *construct_parse_tree(Parser *parser) {
    log_num = 0;
    parse_tree_number_of_nodes = 0;
    parse_tree_allocated_memory = 0;
    parser_log = fopen(PARSER_LOG_FILE, "w");
    Lexer *lexer = parser->lexer;
    const Grammar *grammar = parser->grammar;

    Stack *semantic_stack = create_stack(sizeof(void *));

    // Stack of TreeNodePtrs (TNPs)
    Stack *stack = create_stack(sizeof(TreeNodePtr));
    int last_syntax_error_printed_on_line = 0;

    int DOLLAR_SYMBOL = DOLLAR;
    int START_SYMBOL = Pro;

    HashSet *common_syn_set = get_common_syn_set(grammar);


    ParseTree *tree = malloc_safe(sizeof(ParseTree));
    TreeNodePtr dollar_node = create_empty_tree_node_for(DOLLAR_SYMBOL);
    push_stack(stack, &dollar_node); // pass address of dollar node as stack is of TreeNodePtrs
    TreeNodePtr start_node = create_empty_tree_node_for(START_SYMBOL);
    push_stack(stack, &start_node); // pass address of start node as stack is of TreeNodePtrs
    start_node->parent_symbol = ROOT_PARENT;
    tree->root = start_node; // set root which is a TNP
    print_stack_to_log(stack);
    const TreeNodePtr *top_of_stack = peek_stack(stack); // peek_stack returns address of data inside
    Token *token = get_next_token(lexer);
    while (stack->size > 0 && (*top_of_stack)->symbol != DOLLAR_SYMBOL && token != NULL) {
        int top_of_stack_symbol = (*top_of_stack)->symbol;
        fprintf(parser_log, "TOKEN = %s\n", token->lexeme);
        ++log_num;
        if (is_terminal(top_of_stack_symbol)) {
            token = handle_terminal_on_top(stack, lexer, top_of_stack, token,
                                           &last_syntax_error_printed_on_line);
        } else if (is_non_terminal(top_of_stack_symbol)) {
            // CASE: NON-TERMINAL CASE ON TOP OF STACK
            token = handle_non_terminal_on_top(stack, parser, lexer, grammar, top_of_stack, token, common_syn_set,
                                               &last_syntax_error_printed_on_line, semantic_stack);
        } else if (is_action(top_of_stack_symbol) &&
                   last_syntax_error_printed_on_line == 0) //do  nothing, should be stored as a node
        {
            // abort AST creation on error;
            execute_action_on_semantic_stack(semantic_stack, top_of_stack_symbol, lexer);
            pop_stack(stack, &top_of_stack);
        } else {
            puts("Unknown");
            exit(1);
        }
        print_stack_to_log(stack);
        top_of_stack = peek_stack(stack);
    }

    fclose(parser_log);
    //tree->semantic_stack = semantic_stack;
    tree->semantic_stack = semantic_stack;
    tree->got_syntax_error = last_syntax_error_printed_on_line == 0 ? false : true;
    if (!tree->got_syntax_error) {
        ASTNode *program_node;
        pop_stack(semantic_stack, &program_node);
        tree->program_node = (ProgramNode *) program_node;
    } else {
        tree->program_node = NULL;
    }
    while (get_next_token(lexer) != NULL) {
        token = get_next_token(lexer);
        free(token);
    }
    return tree;
}

static void destroy_parse_tree(ParseTree *parse_tree) {
    destroy_parse_tree_node(parse_tree->root);
    free(parse_tree);
}

static void destroy_parse_tree_node(TreeNode *node) {
    for (size_t i = 0; i < node->children->size; ++i) {
        TreeNodePtr *tree_node_ptr = get_element_at_index_arraylist(node->children, i);
        destroy_parse_tree_node(*tree_node_ptr);
    }
    destroy_arraylist(node->children);
}


/**
 * Handles terminal on top of stack
 * returns true if no error (i.e. terminal matches token)
 * returns false if an error was encountered
 * Pops the top of stack in both cases
 */
static Token *
handle_terminal_on_top(Stack *stack, Lexer *lexer, TreeNodePtr const *top_of_stack, Token *token,
                       int *last_syntax_error_line) {
    int top_of_stack_symbol = (*top_of_stack)->symbol;
    // TODO: Can convert to simple int comparison as terminal and token range is same
    const char *terminal_string = SYMBOL_TO_STRING[top_of_stack_symbol];
    // TODO: Edit to term_to_str map
    const char *token_type_string = SYMBOL_TO_STRING[token->token_type];

    // pop from stack regardless of error
    TreeNodePtr popped;
    pop_stack(stack, &popped);
    popped->popped_from_stack = true;
    popped->token = token;

    if (top_of_stack_symbol != token->token_type) {
        /* Observe that popped is TreeNodePtr, so it needs to have the correct token-type in the Parse Tree
         * But we received a token type with "typo" from input stream
         * type to the expected type (i.e. the terminal of the grammar just popped from the stack)
         * Summary: Fix the "typo" in input stream with expected type
         */
        // Todo: Print program not PROGRAM (i.e. token)
        if (token->line_number != *last_syntax_error_line) {
            *last_syntax_error_line = token->line_number;
            char error_message[256];
            snprintf(error_message, sizeof error_message,
                     "Syntax Error on Line #%d: Expected Token: %s, Found: %s [%s]\n",
                     token->line_number, terminal_string, token_type_string, token->lexeme);
            report_error(SYNTAX_ERROR, error_message);
        }
        return token;
    }
    return get_next_token(lexer);
}


static void print_stack_to_log(Stack *parser_stack) {
    fprintf(parser_log, "STACK AT THIS INSTANT %d: ", log_num);
    LinkedList *ll = parser_stack->linked_list;
    size_t size = ll->size;
    LinkedListNode *current = ll->sentinel->next;
    for (size_t i = 0; i < size; ++i) {
        // as stack is of TreeNodePtrs
        TreeNodePtr *tnp = current->data;
        int symbol = (*tnp)->symbol;
        fprintf(parser_log, "%s => ", SYMBOL_TO_STRING[symbol]);
        current = current->next;
    }
    fprintf(parser_log, "\n");
}


Token *
handle_non_terminal_on_top(Stack *stack, const Parser *parser, Lexer *lexer, const Grammar *grammar,
                           TreeNodePtr const *top_of_stack, Token *token, HashSet *common_syn_set,
                           int *last_syntax_error_line, Stack *semantic_stack) {
    int non_terminal_symbol = (*top_of_stack)->symbol;

    Production ***parse_table = parser->parse_table;
    // NON TERMINAL BRANCH
    Production *current = parse_table[non_terminal_symbol][token->token_type];
    if (current == NULL) {
//        printf("Symbol %d", NT);
//        print_symbol(grammar, &NT);
//        puts("^^^^");
        // if whichAssignment, pop till stmt is popped and consume ip stream till semicolon is found
        if (non_terminal_symbol == whichAssignment) {
            const TreeNodePtr *peeker = peek_stack(stack);
            while (stack->size > 0 && (*peeker)->symbol != statements) {
                TreeNodePtr popped;
                pop_stack(stack, &popped);
                popped->popped_from_stack = true;
                peeker = peek_stack(stack);
            }
            while (token->token_type != SEMICOLON) {
                // Semicolon
                token = get_next_token(lexer);
            }
            token = get_next_token(lexer);
            return token;
        }


        // In error cell of parse table
        HashSet **first_of_current_non_terminal = get_from_hashmap(grammar->first_sets_map, &non_terminal_symbol);
        HashSet **follow_of_current_non_terminal = get_from_hashmap(grammar->follow_sets_map, &non_terminal_symbol);
        HashSet *synchronization_set = hashset_union(*first_of_current_non_terminal,
                                                     *follow_of_current_non_terminal);
        add_all_from_hashset(common_syn_set, synchronization_set);
        int semicolon = SEMICOLON;
        add_to_hashset(synchronization_set, &semicolon);

        Token *to_report_in_error = token;
        while ((token = get_next_token(lexer)) != NULL) {
            if (!hashset_contains(synchronization_set, &token->token_type)) {
                continue;
            }
            destroy_hashset(synchronization_set);
            if (hashset_contains(*first_of_current_non_terminal, &token->token_type)) {
                // First set contains the current token of input stream,
                // Case: Programmer wrote extra stuff in between current and to_rep_in_error token
                // TODO: Earlier this was to_rep_in_err with lexeme & line number
                if (*last_syntax_error_line != token->line_number) {
                    char error_message[256];
                    sprintf(error_message, "Syntax Error: Unexpected token: %s [%s] on line number %d\n",
                            SYMBOL_TO_STRING[token->token_type],
                            token->lexeme,
                            token->line_number);
                    report_error(SYNTAX_ERROR, error_message);
                    *last_syntax_error_line = token->line_number;
                }
                return token;
            }
            // Follow set contains the current token or current token is epsilon
            // Case: Programmer missed typing stuff. Indicate incomplete expression type
            if (*last_syntax_error_line != token->line_number) {
                *last_syntax_error_line = token->line_number;
                char error_message[256];
                sprintf(error_message, "Syntax Error: Unexpected token: %s [%s] on line number %d. Incomplete %s\n",
                        SYMBOL_TO_STRING[to_report_in_error->token_type],
                        to_report_in_error->lexeme,
                        to_report_in_error->line_number,
                        SYMBOL_TO_STRING[non_terminal_symbol]);
                report_error(SYNTAX_ERROR, error_message);
            }
            TreeNodePtr *popped;
            pop_stack(stack, &popped);
            return token;
        }
        if (token == NULL || *last_syntax_error_line != token->line_number) {
            char error_message[256];
            sprintf(error_message, "Syntax Error: Unexpected token: %s [%s] on line number %d\n",
                    SYMBOL_TO_STRING[to_report_in_error->token_type],
                    to_report_in_error->lexeme,
                    to_report_in_error->line_number);
            if (token != NULL)
                *last_syntax_error_line = token->line_number;
            report_error(SYNTAX_ERROR, error_message);
        }
        destroy_hashset(synchronization_set);
        return token;
    }


    // Production cell is not "error" labelled
    // print_production(grammar, current);
    TreeNodePtr popped;
    pop_stack(stack, &popped);
    popped->popped_from_stack = true;

    if (!is_null_production(current)) {
        TreeNodePtr children[current->rhs_size];
        for (int i = current->rhs_size - 1; i >= 0; --i) {
            TreeNodePtr tree_node_for_current_rhs = create_empty_tree_node_for(current->rhs[i]);
            tree_node_for_current_rhs->parent_symbol = popped->symbol;
            children[i] = tree_node_for_current_rhs;
            push_stack(stack, &tree_node_for_current_rhs);
        }
        for (int i = 0; i < current->rhs_size; ++i) {
            add_element_arraylist(popped->children, &children[i]);
        }
    } else {

        TreeNodePtr epsilon = create_empty_tree_node_for(EPSILON);
        epsilon->parent_symbol = popped->symbol;
        epsilon->token = create_epsilon_token(token->line_number);
        epsilon->popped_from_stack = true;

        // Push actions if applicable:
        for (int i = current->rhs_size - 1; i >= 0; --i) {
            if (is_action(current->rhs[i])) {
                TreeNodePtr n = create_empty_tree_node_for(current->rhs[i]);
                push_stack(stack, &n);
            }
        }

        add_element_arraylist(popped->children, &epsilon);
    }

    return token;
}

static Token *create_epsilon_token(int line_number) {
    Token *t = malloc_safe(sizeof(Token));
    t->lexeme = "";
    t->token_type = EPSILON;
    t->line_number = line_number;
    return t;
}


static TreeNodePtr create_empty_tree_node_for(int symbol) {
    parse_tree_number_of_nodes += 1;
    parse_tree_allocated_memory += sizeof(TreeNode);
    TreeNodePtr t = malloc_safe(sizeof(TreeNode));
    t->symbol = symbol;
    t->token = NULL;
    t->children = create_arraylist(sizeof(TreeNode *));
    t->popped_from_stack = false;
    return t;
}

void print_parse_tree(Parser *parser, ParseTree *parse_tree, FILE *fp) {

    if (!fp) {
        //   report_file_error(path);
        return;
    }
    if (parse_tree == NULL) {
        return;
    }
    fprintf(fp, "%-25s %-10s %-15s %-20s %-15s %-10s %-10s\n",
            "Lexeme", "Line #", "Token Type",
            "Value (If Number)", "Parent Node", "Is Leaf", "Node Symbol");
    print_tree_node(parser, parse_tree->root, fp);
}


static void print_tree_node(Parser *parser, TreeNodePtr node, FILE *fp) {
    size_t num_children = get_arraylist_size(node->children);
    for (size_t i = 0; i < get_arraylist_size(node->children); ++i) {
        const TreeNodePtr *ptr = get_element_at_index_arraylist(node->children, i);
        if (!(*ptr)->popped_from_stack)
            --num_children;
    }
    if (num_children == 0) {
        if (is_terminal(node->symbol)) { format_and_print_leaf_terminal(node, fp); }
        else if (is_non_terminal(node->symbol)) {
            format_and_print_leaf_non_terminal(node, fp);
        }
        return;
    }
    TreeNodePtr *left_child = get_first_arraylist(node->children);
    print_tree_node(parser, *left_child, fp);
    format_and_print_non_leaf_non_terminal(node, fp);
    for (size_t i = 1; i < num_children; ++i) {
        const TreeNodePtr *right_i_child = get_element_at_index_arraylist(node->children, i);
        print_tree_node(parser, *right_i_child, fp);
    }
}


static void format_and_print_leaf_terminal(TreeNodePtr tree_node_ptr, FILE *fp) {
    Token *t = tree_node_ptr->token;
    if (t == NULL) {
        // This might be a terminal in stack that never got the chance to be popped but its parent node contains
        // a reference to it
        fprintf(stdout, "Invalid leaf terminal\n");
        return;
    }
    char *end;
    if (strcmp("NUM", SYMBOL_TO_STRING[tree_node_ptr->token->token_type]) == 0) {
        long l_val = strtol(tree_node_ptr->token->lexeme, &end, 10);
        fprintf(fp, "%-25s %-10d %-15s %-20ld %-15s %-10s %-10s\n",
                t->lexeme, t->line_number, SYMBOL_TO_STRING[t->token_type],
                l_val,
                SYMBOL_TO_STRING[tree_node_ptr->parent_symbol],
                "Yes", SYMBOL_TO_STRING[t->token_type]);
        return;
    } else if (strcmp("RNUM", SYMBOL_TO_STRING[tree_node_ptr->token->token_type]) == 0) {
        double d_val = strtod(tree_node_ptr->token->lexeme, &end);
        fprintf(fp, "%-25s %-10d %-15s %-20f %-15s %-10s %-10s\n",
                t->lexeme, t->line_number, SYMBOL_TO_STRING[t->token_type],
                d_val,
                SYMBOL_TO_STRING[tree_node_ptr->parent_symbol],
                "Yes", SYMBOL_TO_STRING[t->token_type]);
        return;
    }

    fprintf(fp, "%-25s %-10d %-15s %-20s %-15s %-10s %-10s\n",
            t->lexeme, t->line_number, SYMBOL_TO_STRING[t->token_type],
            " ",
            SYMBOL_TO_STRING[tree_node_ptr->parent_symbol],
            "Yes", SYMBOL_TO_STRING[t->token_type]);
}

static void format_and_print_leaf_non_terminal(TreeNodePtr node, FILE *fp) {
    // Handle case where root is the leaf for erroneous parse tree
    const char *parent_string = node->parent_symbol == ROOT_PARENT ? "Root" : SYMBOL_TO_STRING[node->parent_symbol];
    fprintf(fp, "%-25s %-10s %-15s %-20s %-15s %-10s %-10s\n", "------", "", " ", " ",
            parent_string,
            "Yes",
            SYMBOL_TO_STRING[node->symbol]);
}

static void format_and_print_non_leaf_non_terminal(TreeNodePtr node, FILE *fp) {
    int START_SYMBOL = Pro;
    const char *string = node->symbol == START_SYMBOL ? "Root" : SYMBOL_TO_STRING[node->parent_symbol];
    fprintf(fp, "%-25s %-10s %-15s %-20s %-15s %-10s %-10s\n", "------", "", " ", " ", string, "No",
            SYMBOL_TO_STRING[node->symbol]);
}
