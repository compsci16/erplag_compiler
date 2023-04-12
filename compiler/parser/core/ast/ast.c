#include <string.h>
#include "utility.h"
#include "stack/stack.h"
#include "arraylist.h"
#include "core/lexer.h"
#include "core/ast/nodes/print_statement.h"
#include "core/ast/nodes/integer_literal.h"
#include "grammar/grammar_populator/grammar_constants.h"
#include "core/ast/nodes/get_val.h"
#include "core/ast/nodes/program.h"
#include "core/ast/nodes/real_literal.h"
#include "core/ast/nodes/boolean_literal.h"
#include "core/ast/nodes/module_declarations.h"
#include "core/ast/nodes/module_node.h"
#include "core/ast/nodes/variable_list.h"
#include "core/ast/nodes/variable_node.h"
#include "core/ast/nodes/switch.h"
#include "core/ast/nodes/case.h"
#include "core/ast/nodes/statements.h"
#include "core/ast/nodes/array_variable.h"
#include "core/ast/nodes/for.h"
#include "core/ast/nodes/binary_operator.h"
#include "core/ast/nodes/id.h"
#include "core/ast/nodes/array.h"
#include "core/ast/nodes/unary_operator.h"
#include "core/ast/nodes/declare_variable_statement.h"
#include "core/ast/nodes/assignment_statement.h"
#include "core/ast/nodes/declare_array_statement.h"
#include "ast.h"
#include "core/ast/nodes/while.h"
#include "core/ast/nodes/module_use_statement.h"
#include <stdlib.h>


char *AST_LOG = "./ast_log.txt";
FILE *ast_log_file = NULL;
int global_action;
Stack *global_semantic_stack;

static void print_semantic_stack();

static void print_log_push(void *ast_node);

static void print_log_pop(void *ast_node);

static void print_log_push(void *ast_node) {
    ASTNode *ast_node_p = ast_node;
    fprintf(ast_log_file, "%s: ", SYMBOL_TO_STRING[global_action]);
    fprintf(ast_log_file, "Pushed %s\n", ast_node_p->name);
}

static void print_log_pop(void *ast_node) {
    ASTNode *ast_node_p = ast_node;
    fprintf(ast_log_file, "%s: ", SYMBOL_TO_STRING[global_action]);
    fprintf(ast_log_file, "Popped %s\n", ast_node_p->name);
}

static void print_semantic_stack() {
    LinkedList *ll = global_semantic_stack->linked_list;
    LinkedListNode *head = ll->sentinel->next;
    size_t size = ll->size;
    for (size_t i = 0; i < size; ++i) {
        ASTNode *const *node = head->data;
        fprintf(ast_log_file, "%s =>", (*node)->name);
        head = head->next;
    }
    fprintf(ast_log_file, "\n");
}

// Semantic stack - contains pointers to AST Nodes
void execute_action_on_semantic_stack(Stack *semantic_stack, int action, Lexer *lexer) {
    if (!ast_log_file) {
        ast_log_file = fopen(AST_LOG, "w");
    }
    global_action = action;
    global_semantic_stack = semantic_stack;
    void *popped;
    switch (action) {
        case ACTION1: {
            StatementsNode *statements_node = create_statements_node();
            pop_stack(semantic_stack, &popped);
            ProgramNode *program_node = popped;
            ModuleNode *module_node = create_module_node();
            module_node->module_id = create_id_node_with_token(create_token_with_type_and_lexeme_for(DRIVER));
            module_node->start_line = get_current_token(lexer)->line_number;
            module_node->output_parameter_list_node = create_variable_list_node();
            module_node->input_parameter_list_node = create_variable_list_node();
            program_node->driver_module = module_node;
            push_stack(semantic_stack, &popped);
//            void *sn = statements_node;
//            push_stack(semantic_stack, &sn);
            push_stack(semantic_stack, &(void *) {statements_node});
            print_log_push(statements_node);
            break;
        }
        case ACTION2: {
            GetValNode *gv_node = create_get_val_statement_node();
            push_stack(semantic_stack, &(void *) {gv_node});
            print_log_push(gv_node);
            break;
        }
        case ACTION3: {
            /*
             * 1. Get_Value (gv_node) node is at the top of stack at this instant, pop it. Now statements node is at top
             * 2. populate the id field of gv_node
             * 3. Pop StmtsNode, add gv_node to it's list of statements
             * 4. Push StmtsNode back
             */
            pop_stack(semantic_stack, &popped);
            GetValNode *gv_node = popped;
            print_log_pop(gv_node);
            gv_node->id = create_id_node_with_token(lexer->current_token);

            pop_stack(semantic_stack, &popped);
            StatementsNode *stmts_node = popped;
            print_log_pop(stmts_node);
            add_element_arraylist(stmts_node->statements_list, &gv_node);
            push_stack(semantic_stack, &(void *) {stmts_node});
            print_log_push(stmts_node);
            break;
        }
        case ACTION4: {
            // Push Program i.e. the AST Root Node to the semantic stack
            ProgramNode *program_node = create_program_node();
            push_stack(semantic_stack, &(void *) {program_node});
            print_log_push(program_node);
            break;
        }
        case ACTION5: {
            // Finished reading driver module definition, now driver_stmts node must be at the top
            // pop it and add it as a child to program node
            pop_stack(semantic_stack, &popped);
            StatementsNode *driver_statements_node = popped;
            print_log_pop(driver_statements_node);
            pop_stack(semantic_stack, &popped);
            ProgramNode *program_node = popped;
            print_log_pop(program_node);
            program_node->driver_module->statements_node = driver_statements_node;
            program_node->driver_module->end_line = get_current_token(lexer)->line_number;
            push_stack(semantic_stack, &(void *) {program_node});
            print_log_push(program_node);
            break;
        }
        case ACTION6: {
            // Push print_node to semantic_stack
            // statements node must be at the top of the stack as PRINT is a new statement
            // Push the Print_Statement_Node to stack
            PrintStatementNode *print_statement_node = create_print_statement_node();
            push_stack(semantic_stack, &(void *) {print_statement_node});
            print_log_push(print_statement_node);
            break;
        }
        case ACTION7: {
            // RULE: itemInPrint = 507,NUM,0
            // so expecting print_statement_node at top
            pop_stack(semantic_stack, &popped);
            PrintStatementNode *print_statement_node = popped;
            print_log_pop(print_statement_node);
            // Now create Integer Literal Node and add it as print specific data
            IntegerLiteralNode *integer_literal_node = create_integer_literal_node();
            integer_literal_node->num = lexer->current_token;
            print_statement_node->print_item = (ASTNode *) integer_literal_node;
            // Now print_statement is complete, driverStatements node must be at the top
            pop_stack(semantic_stack, &popped);
            StatementsNode *driver_statements_node = popped;
            print_log_pop(driver_statements_node);
            // Add print element to the list of driver statements;
            add_element_arraylist(driver_statements_node->statements_list, &print_statement_node);

            // Push driver statements node back
            push_stack(semantic_stack, &(void *) {driver_statements_node});
            print_log_push(driver_statements_node);
            break;
        }
        case ACTION8: {
            // RULE: itemInPrint = 508,RNUM,0
            // so expecting print_statement_node at top
            pop_stack(semantic_stack, &popped);
            PrintStatementNode *print_statement_node = popped;
            print_log_pop(print_statement_node);
            // Now create Real Literal Node and add it as print specific data
            RealLiteralNode *real_literal_node = create_real_literal_node();
            real_literal_node->r_num = lexer->current_token;
            print_statement_node->tag = REAL_LITERAL;
            print_statement_node->print_item = (ASTNode *) real_literal_node;
            // Now print_statement is complete, driverStatements node must be at the top

            pop_stack(semantic_stack, &popped);
            StatementsNode *driver_statements_node = popped;
            print_log_pop(driver_statements_node);
            // Add print element to the list of driver statements;
            add_element_arraylist(driver_statements_node->statements_list, &print_statement_node);

            // Push driver statements node back
            push_stack(semantic_stack, &(void *) {driver_statements_node});
            print_log_push(driver_statements_node);
            break;
        }
        case ACTION9: {
            // RULE: itemInPrint = 509,booleanConstant,0
            // so expecting print_statement_node at top
            pop_stack(semantic_stack, &popped);
            PrintStatementNode *print_statement_node = popped;
            print_log_pop(print_statement_node);
            // Now create Boolean Literal Node and add it as print specific data
            BooleanLiteralNode *boolean_literal_node = create_boolean_literal_node();
            boolean_literal_node->boolean_const = lexer->current_token;
            print_statement_node->tag = BOOLEAN_LITERAL;
            print_statement_node->print_item = (ASTNode *) boolean_literal_node;
            // Now print_statement is complete, driverStatements node must be at the top


            pop_stack(semantic_stack, &popped);
            StatementsNode *driver_statements_node = popped;
            print_log_pop(driver_statements_node);

            // Add print element to the list of driver statements;
            add_element_arraylist(driver_statements_node->statements_list, &print_statement_node);

            // Push driver statements node back
            push_stack(semantic_stack, &(void *) {driver_statements_node});
            print_log_push(driver_statements_node);
            break;
        }
        case ACTION10: {
            // prepare switch node and push it
            SwitchStatementNode *switch_node = create_switch_node();
            push_stack(semantic_stack, &(void *) {switch_node});
            break;
        }
        case ACTION11: {
            // switch node at top, pop it, populate its id field
            pop_stack(semantic_stack, &popped);
            SwitchStatementNode *switch_node = popped;
            print_log_pop(switch_node);
            switch_node->id = create_id_node_with_token(lexer->current_token);
            push_stack(semantic_stack, &(void *) {switch_node});
            print_log_push(switch_node);
            break;
        }
        case ACTION12: {
            // switch ends
            pop_stack(semantic_stack, &popped);
            SwitchStatementNode *switch_node = popped;
            switch_node->end_line = get_current_token(lexer)->line_number;
            print_log_pop(switch_node);
            pop_stack(semantic_stack, &popped);
            StatementsNode *statements_node = popped;
            print_log_pop(statements_node);
            add_element_arraylist(statements_node->statements_list,
                                  &switch_node);
            push_stack(semantic_stack, &(void *) {statements_node});
            print_log_push(statements_node);
            break;
        }
        case ACTION13: {
            CaseStatementNode *case_node = create_case_node();
            case_node->start_line = get_current_token(lexer)->line_number;
            push_stack(semantic_stack, &(void *) {case_node});
            print_log_push(case_node);
            break;
        }
        case ACTION14: {
            // Pop case node from top of stack and set its case tag to NUM
            pop_stack(semantic_stack, &popped);
            CaseStatementNode *case_node = popped;
            print_log_pop(case_node);
            case_node->tag = INTEGER_CONSTANT;
            case_node->case_value = lexer->current_token;
            push_stack(semantic_stack, &(void *) {case_node});
            print_log_push(case_node);
            break;
        }
        case ACTION15: {
            pop_stack(semantic_stack, &popped);
            CaseStatementNode *case_node = popped;
            print_log_pop(case_node);
            case_node->tag = BOOLEAN_CONSTANT;
            case_node->case_value = lexer->current_token;
            push_stack(semantic_stack, &(void *) {case_node});
            print_log_push(case_node);
            break;
        }
        case ACTION16: {
            StatementsNode *statements_node = create_statements_node();
            push_stack(semantic_stack, &(void *) {statements_node});
            print_log_push(statements_node);
            break;
        }
        case ACTION17: {
            pop_stack(semantic_stack, &popped);
            StatementsNode *statements_node = popped;
            print_log_pop(statements_node);
            pop_stack(semantic_stack, &popped);
            CaseStatementNode *case_node = popped;
            print_log_pop(case_node);
            case_node->statements_node = statements_node;
            case_node->end_line = get_current_token(lexer)->line_number;
            pop_stack(semantic_stack, &popped);
            SwitchStatementNode *switch_node = popped;
            print_log_pop(switch_node);
            add_element_arraylist(switch_node->case_list, &case_node);
            push_stack(semantic_stack, &(void *) {switch_node});
            print_log_push(switch_node);
            break;
        }
        case ACTION18: {
            CaseStatementNode *dft_case_statement_node = create_case_node();
            dft_case_statement_node->start_line = get_current_token(lexer)->line_number;
            push_stack(semantic_stack, &(void *) {dft_case_statement_node});
            StatementsNode *default_statements_node = create_statements_node();
            push_stack(semantic_stack, &(void *) {default_statements_node});
            print_log_push(default_statements_node);
            break;
        }
        case ACTION19: {
            pop_stack(semantic_stack, &popped);
            StatementsNode *default_statements_node = popped;
            print_log_pop(default_statements_node);
            pop_stack(semantic_stack, &popped);
            CaseStatementNode *default_case = popped;
            default_case->end_line = get_current_token(lexer)->line_number;
            default_case->statements_node = default_statements_node;
            pop_stack(semantic_stack, &popped);
            SwitchStatementNode *switch_node = popped;
            print_log_pop(switch_node);
            switch_node->default_case_statement_node = default_case;
            push_stack(semantic_stack, &(void *) {switch_node});
            print_log_push(switch_node);
            break;
        }
        case ACTION20: {
            // Make module declarations node and push it
            ModuleDeclarationsNode *module_declarations_node = create_module_declarations_node();

            // Pop program and set its moduleDeclarations field
            pop_stack(semantic_stack, &popped);
            ProgramNode *program_node = popped;
            print_log_pop(program_node);
            program_node->module_declarations_node = module_declarations_node;

            // Push program back
            push_stack(semantic_stack, &(void *) {program_node});
            print_log_push(program_node);
            break;
        }
        case ACTION21: {
            // Pop program node and add the current id to its module declarations field's list
            pop_stack(semantic_stack, &popped);
            ProgramNode *program_node = popped;
            print_log_pop(program_node);
            IDNode *id_node = create_id_node_with_token(lexer->current_token);
            ModuleDeclarationsNode *module_declarations_node = program_node->module_declarations_node;
            add_element_arraylist(module_declarations_node->module_ids, &id_node);
            // Push program node back
            push_stack(semantic_stack, &(void *) {program_node});
            print_log_push(program_node);
            break;
        }
        case ACTION22: {
            // Found new module, push it to stack. Semantic Stack looks like; ProgramNode ModuleNode
            ModuleNode *module_node = create_module_node();
            push_stack(semantic_stack, &(void *) {module_node});
            print_log_push(module_node);
            break;
        }
        case ACTION23: {
            // Found module name, populate it:
            // module = DEF,522,MODULE,523,ID,ENDDEF,TAKES,INPUT,SQBO,Ipl,SQBC,SEMICOLON,returnList,moduleDef,0
            pop_stack(semantic_stack, &popped);
            ModuleNode *module_node = popped;
            print_log_pop(module_node);
            module_node->module_id = create_id_node_with_token(lexer->current_token);
            push_stack(semantic_stack, &(void *) {module_node});
            print_log_push(module_node);
            break;
        }
        case ACTION24: {
            // module = DEF,522,MODULE,523,ID,ENDDEF,TAKES,INPUT,524,SQBO,Ipl,SQBC,SEMICOLON,returnList,moduleDef,0
            // Push IPL
            VariableListNode *parameter_list = create_variable_list_node();
            push_stack(semantic_stack, &(void *) {parameter_list});
            print_log_push(parameter_list);
            break;
        }
        case ACTION25: {
            // Create and Push variable node (note that it can be an array_variable node too, so we might have
            // to pop and recreate this node later on seeing the keyword array)
            VariableNode *variable_node = create_variable_node();
            variable_node->id = create_id_node_with_token(lexer->current_token);
            push_stack(semantic_stack, &(void *) {variable_node});
            print_log_push(variable_node);
            break;
        }
        case ACTION26: {
            // Pop variable node, set its data type
            pop_stack(semantic_stack, &popped);
            VariableNode *variable_node = popped;
            print_log_pop(variable_node);
            variable_node->type = lexer->current_token;
            push_stack(semantic_stack, &(void *) {variable_node});
            print_log_push(variable_node);
            break;
        }
        case ACTION27: {
            // Found array, pop variable and get its id; Set that to array's id and push array node to stack
            pop_stack(semantic_stack, &popped);
            VariableNode *variable_node = popped;
            print_log_pop(variable_node);
            ArrayVariableNode *array_variable_node = create_array_variable_node();
            array_variable_node->id = variable_node->id;
            push_stack(semantic_stack, &(void *) {array_variable_node});
            print_log_push(array_variable_node);
            free(variable_node);
            break;
        }
        case ACTION28: {
            // Pop array node, set it's begin range sign
            pop_stack(semantic_stack, &popped);
            ArrayVariableNode *array_variable_node = popped;
            print_log_pop(array_variable_node);
            // Has sign?
            if (lexer->current_token->token_type == PLUS || lexer->current_token->token_type == MINUS) {
                array_variable_node->array_range.begin_sign = get_current_token(
                        lexer);
            } else {
                Token *plus = malloc_safe(sizeof(Token));
                plus->lexeme = "+";
                plus->token_type = PLUS;
                array_variable_node->array_range.begin_sign = plus;
            }
            push_stack(semantic_stack, &(void *) {array_variable_node});
            print_log_push(array_variable_node);
            break;
        }
        case ACTION29: {
            // set array node's begin sign
            pop_stack(semantic_stack, &popped);
            ArrayVariableNode *array_variable_node = popped;
            print_log_pop(array_variable_node);
            array_variable_node->array_range.begin = lexer->current_token;
            push_stack(semantic_stack, &(void *) {array_variable_node});
            print_log_push(array_variable_node);
            break;
        }
        case ACTION30: {
            // set array node's end sign
            pop_stack(semantic_stack, &popped);
            ArrayVariableNode *array_variable_node = popped;
            print_log_pop(array_variable_node);
            if (lexer->current_token->token_type == PLUS || lexer->current_token->token_type == MINUS) {
                array_variable_node->array_range.end_sign = get_current_token(
                        lexer);
            } else {
                Token *plus = malloc_safe(sizeof(Token));
                plus->lexeme = "+";
                plus->token_type = PLUS;
                array_variable_node->array_range.end_sign = plus;
            }
            push_stack(semantic_stack, &(void *) {array_variable_node});
            print_log_push(array_variable_node);
            break;
        }
        case ACTION31: {
            // set array node's end
            pop_stack(semantic_stack, &popped);
            ArrayVariableNode *array_variable_node = popped;
            print_log_pop(array_variable_node);
            array_variable_node->array_range.end = lexer->current_token;
            push_stack(semantic_stack, &(void *) {array_variable_node});
            print_log_push(array_variable_node);
            break;
        }
        case ACTION32: {
            // one variable/array variable declaration of input parameter list completed so add it
            pop_stack(semantic_stack, &popped);
            ASTNode *var_node = popped;
            print_log_pop(var_node);
            pop_stack(semantic_stack, &popped);
            VariableListNode *input_parameter_list = popped;
            print_log_pop(input_parameter_list);
            add_element_arraylist(input_parameter_list->variable_nodes_list, &var_node);
            push_stack(semantic_stack, &(void *) {input_parameter_list});
            print_log_push(input_parameter_list);
            break;
        }
        case ACTION33: {
            pop_stack(semantic_stack, &popped);
            VariableListNode *input_parameter_list_node = popped;
            print_log_pop(input_parameter_list_node);
            pop_stack(semantic_stack, &popped);
            ModuleNode *module_node = popped;
            print_log_pop(module_node);
            module_node->input_parameter_list_node = input_parameter_list_node;
            push_stack(semantic_stack, &(void *) {module_node});
            print_log_push(module_node);
            break;
        }
        case ACTION34: {
            VariableListNode *output_parameter_list = create_variable_list_node();
            push_stack(semantic_stack, &(void *) {output_parameter_list});
            print_log_push(output_parameter_list);
            break;
        }
        case ACTION35: {
            // Returns => EPSILON , so just push empty return list node
            VariableListNode *output_parameter_list = create_variable_list_node();
            pop_stack(semantic_stack, &popped);
            ModuleNode *module_node = popped;
            print_log_pop(module_node);
            module_node->output_parameter_list_node = output_parameter_list;
            push_stack(semantic_stack, &(void *) {module_node});
            print_log_push(module_node);
            break;
        }
        case ACTION36: {
            VariableNode *variable_node = create_variable_node(); //added create variable node
            variable_node->id = create_id_node_with_token(lexer->current_token);
            push_stack(semantic_stack, &(void *) {variable_node});
            print_log_push(variable_node);
            break;
        }
        case ACTION37: {
            pop_stack(semantic_stack, &popped);
            VariableNode *variable_node = popped;
            print_log_pop(variable_node);
            variable_node->type = lexer->current_token;
            push_stack(semantic_stack, &(void *) {variable_node});
            print_log_push(variable_node);
            break;
        }
        case ACTION38: {
            pop_stack(semantic_stack, &popped);
            VariableNode *variable_node = popped;
            print_log_pop(variable_node);
            pop_stack(semantic_stack, &popped);
            VariableListNode *output_parameter_list_node = popped;
            print_log_pop(output_parameter_list_node);
            add_element_arraylist(output_parameter_list_node->variable_nodes_list, &variable_node);
            push_stack(semantic_stack, &(void *) {output_parameter_list_node});
            print_log_push(output_parameter_list_node);
            break;
        }
        case ACTION39: {
            pop_stack(semantic_stack, &popped);
            VariableListNode *output_parameter_list_node = popped;
            print_log_pop(output_parameter_list_node);
            pop_stack(semantic_stack, &popped);
            ModuleNode *module_node = popped;
            print_log_pop(module_node);
            module_node->output_parameter_list_node = output_parameter_list_node;
            push_stack(semantic_stack, &(void *) {module_node});
            print_log_push(module_node);
            break;
        }
        case ACTION40: {
            pop_stack(semantic_stack, &popped);
            ModuleNode *module_node = popped;
            print_log_pop(module_node);
            module_node->start_line = get_current_token(lexer)->line_number;
            push_stack(semantic_stack, &(void *) {module_node});
            // push module statements
            StatementsNode *statements_node = create_statements_node();
            push_stack(semantic_stack, &(void *) {statements_node});
            print_log_push(statements_node);
            break;
        }
        case ACTION41: {
            // module finished
            pop_stack(semantic_stack, &popped);
            StatementsNode *statements_node = popped;
            print_log_pop(statements_node);
            pop_stack(semantic_stack, &popped);
            ModuleNode *module_node = popped;
            print_log_pop(module_node);
            module_node->end_line = get_current_token(lexer)->line_number;
            module_node->statements_node = statements_node;
            pop_stack(semantic_stack, &popped);
            ProgramNode *program_node = popped;
            print_log_pop(program_node);
            add_element_arraylist(program_node->modules_list_node->modules_list, &module_node);
            push_stack(semantic_stack, &(void *) {program_node});
            print_log_push(program_node);
            break;
        }
        case ACTION42: {
            // pop array variable node and set its type
            pop_stack(semantic_stack, &popped);
            ArrayVariableNode *array_variable_node = popped;
            print_log_pop(array_variable_node);
            array_variable_node->type = lexer->current_token;
            push_stack(semantic_stack, &(void *) {array_variable_node});
            print_log_push(array_variable_node);
            break;
        }
        case ACTION43: {
            // create for node, set id of for node
            ForNode *for_node = create_for_node();
            for_node->start_line = get_current_token(lexer)->line_number;
            VariableNode *variable_node = create_variable_node();
            variable_node->id = create_id_node_with_token(lexer->current_token);
            // For can only have integer counter
            Token *integer_type = malloc_safe(sizeof(Token));
            integer_type->lexeme = strdup(SYMBOL_TO_STRING[INTEGER]);
            integer_type->token_type = INTEGER;
            variable_node->type = integer_type;
            for_node->variable_node = variable_node;
            push_stack(semantic_stack, &(void *) {for_node});
            print_log_push(for_node);
            break;
        }
        case ACTION44: {
            pop_stack(semantic_stack, &popped);
            ForNode *for_node = popped;
            print_log_pop(for_node);

            Token *current = lexer->current_token;
            if (current->token_type != PLUS && current->token_type != MINUS) {
                for_node->for_range.begin_sign = create_token_with_type_and_lexeme_for(PLUS);
            } else {
                for_node->for_range.begin_sign = current;
            }
            push_stack(semantic_stack, &(void *) {for_node});
            print_log_push(for_node);
            break;
        }
        case ACTION45: {
            pop_stack(semantic_stack, &popped);
            ForNode *for_node = popped;
            print_log_pop(for_node);
            for_node->for_range.begin = lexer->current_token;
            push_stack(semantic_stack, &(void *) {for_node});
            print_log_push(for_node);
            break;
        }
        case ACTION46: {
            pop_stack(semantic_stack, &popped);
            ForNode *for_node = popped;
            print_log_pop(for_node);
            Token *current = lexer->current_token;
            if (current->token_type != PLUS && current->token_type != MINUS) {
                for_node->for_range.end_sign = create_token_with_type_and_lexeme_for(PLUS);
            } else {
                for_node->for_range.end_sign = current;
            }
            push_stack(semantic_stack, &(void *) {for_node});
            print_log_push(for_node);
            break;
        }
        case ACTION47: {
            pop_stack(semantic_stack, &popped);
            ForNode *for_node = popped;
            print_log_pop(for_node);
            for_node->for_range.end = lexer->current_token;
            push_stack(semantic_stack, &(void *) {for_node});
            print_log_push(for_node);
            StatementsNode *for_statements = create_statements_node();
            push_stack(semantic_stack, &(void *) {for_statements});
            print_log_push(for_statements);
            break;
        }
        case ACTION48: {

            pop_stack(semantic_stack, &popped);
            StatementsNode *for_statements;
            for_statements = popped;
            print_log_pop(for_statements);

            pop_stack(semantic_stack, &popped);
            ForNode *for_node;
            for_node = popped;
            for_node->end_line = get_current_token(lexer)->line_number;
            print_log_pop(for_node);
            for_node->statements_node = for_statements;


            pop_stack(semantic_stack, &popped);
            StatementsNode *enclosing_statements;
            enclosing_statements = popped;
            print_log_pop(enclosing_statements);
            add_element_arraylist(enclosing_statements->statements_list, &for_node);

            push_stack(semantic_stack, &(void *) {enclosing_statements});
            print_log_push(enclosing_statements);
            break;
        }
        case ACTION49: {
            // arithmeticFactor = 549,NUM
            IntegerLiteralNode *integer_literal_factor = create_integer_literal_node();
            integer_literal_factor->num = lexer->current_token;

            pop_stack(semantic_stack, &popped);
            ASTNode *ast_node = popped;
            if (ast_node->tag == NODE_UNARY_OPERATOR &&
                ((UnaryOperatorNode *) ast_node)->unary_tag == UNARY_FACTOR) {
                UnaryOperatorNode *unary_op_node = (UnaryOperatorNode *) ast_node;
                unary_op_node->operand = (ASTNode *) integer_literal_factor;
                push_stack(semantic_stack, &(void *) {unary_op_node});
            } else {
                push_stack(semantic_stack, &(void *) {ast_node});
                push_stack(semantic_stack, &(void *) {integer_literal_factor});
                print_log_push(integer_literal_factor);
            }
            break;
        }
        case ACTION50: {
            RealLiteralNode *real_literal_factor = create_real_literal_node();
            real_literal_factor->r_num = lexer->current_token;
            pop_stack(semantic_stack, &popped);
            ASTNode *ast_node = popped;
            if (ast_node->tag == NODE_UNARY_OPERATOR &&
                ((UnaryOperatorNode *) ast_node)->unary_tag == UNARY_FACTOR) {
                UnaryOperatorNode *unary_operator_node = (UnaryOperatorNode *) ast_node;
                unary_operator_node->operand = (ASTNode *) real_literal_factor;
                push_stack(semantic_stack, &(void *) {unary_operator_node});
            } else {
                push_stack(semantic_stack, &(void *) {ast_node});
                push_stack(semantic_stack, &(void *) {real_literal_factor});
                print_log_push(real_literal_factor);
            }

            break;
        }
        case ACTION51: {
            BooleanLiteralNode *boolean_literal_factor = create_boolean_literal_node();
            boolean_literal_factor->boolean_const = lexer->current_token;

            pop_stack(semantic_stack, &popped);
            ASTNode *ast_node = popped;
            if (ast_node->tag == NODE_UNARY_OPERATOR &&
                ((UnaryOperatorNode *) ast_node)->unary_tag == UNARY_FACTOR) {
                UnaryOperatorNode *unary_op_node = (UnaryOperatorNode *) ast_node;;
                unary_op_node->operand = (ASTNode *) boolean_literal_factor;
                push_stack(semantic_stack, &(void *) {unary_op_node});
            } else {
                push_stack(semantic_stack, &(void *) {ast_node});
                print_log_push(ast_node);
                push_stack(semantic_stack, &(void *) {boolean_literal_factor});
                print_log_push(boolean_literal_factor);
            }
            break;
        }
        case ACTION52: {
            BinaryOperatorNode *operator = create_operator_node();
            operator->operator = lexer->current_token;
            push_stack(semantic_stack, &(void *) {operator});
            print_log_push(operator);
            break;
        }
        case ACTION53: {
            //ZArithmeticExpression = 52,addSubOp,553,arithmeticTerm,554,ZArithmeticExpression
            //ZArithmeticTerm = mulDivOp,553,arithmeticFactor,554,ZArithmeticTerm
            //ZArithmeticOrBooleanExpression = 552,logicalOperator,553,booleanTerm,554,ZArithmeticOrBooleanExpression

            pop_stack(semantic_stack, &popped);
            BinaryOperatorNode *operator;
            operator = popped;
            print_log_pop(operator);

            pop_stack(semantic_stack, &popped);
            ASTNode *arithmetic_factor;
            arithmetic_factor = popped;
            print_log_pop(arithmetic_factor);
            operator->left = arithmetic_factor;
            push_stack(semantic_stack, &(void *) {operator});
            print_log_push(operator);
            break;
        }
        case ACTION54: {
            //ZArithmeticExpression = 552,addSubOp,553,arithmeticTerm,554,ZArithmeticExpression
            //ZArithmeticTerm = mulDivOp,553,arithmeticFactor,554,ZArithmeticTerm
            // Top is factor node, make it the right child of operator node

            pop_stack(semantic_stack, &popped);
            ASTNode *arithmetic_factor;
            arithmetic_factor = popped;
            print_log_pop(arithmetic_factor);

            pop_stack(semantic_stack, &popped);
            BinaryOperatorNode *operator;
            operator = popped;
            print_log_pop(operator);
            operator->right = arithmetic_factor;
            push_stack(semantic_stack, &(void *) {operator});
            print_log_push(operator);
            break;
        }
        case ACTION55: {
            IDNode *id_node = create_id_node();
            id_node->id_token = lexer->current_token;
            pop_stack(semantic_stack, &popped);
            ASTNode *ast_node = popped;
            if (ast_node->tag == NODE_UNARY_OPERATOR &&
                ((UnaryOperatorNode *) ast_node)->unary_tag == UNARY_FACTOR) {
                UnaryOperatorNode *unary_operator_node = (UnaryOperatorNode *) ast_node;
                unary_operator_node->operand = (ASTNode *) id_node;
                push_stack(semantic_stack, &(void *) {unary_operator_node});
            } else {
                push_stack(semantic_stack, &(void *) {ast_node});
                push_stack(semantic_stack, &(void *) {id_node});
                print_log_push(id_node);
            }
            print_semantic_stack();
            break;
        }
        case ACTION56: {
            print_semantic_stack();
            ArrayNode *array_node = create_array_node();
            pop_stack(semantic_stack, &popped);
            IDNode *id_node;
            id_node = popped;
            print_log_pop(id_node);
            array_node->array_id = id_node;
            push_stack(semantic_stack, &(void *) {array_node});
            print_log_push(array_node);
            print_semantic_stack();
            break;
        }
        case ACTION57: {
            pop_stack(semantic_stack, &popped);
            ASTNode *array_index;
            array_index = popped;
            print_log_pop(array_index);

            pop_stack(semantic_stack, &popped);
            ArrayNode *array;
            array = popped;
            print_log_pop(array);
            array->array_expression = array_index;
            push_stack(semantic_stack, &(void *) {array});
            print_log_push(array);
            break;
        }
        case ACTION58: {
            Token *t = lexer->current_token;
            if (t->token_type != PLUS && t->token_type != MINUS) {
                return;
            }
            UnaryOperatorNode *operator_node = create_unary_operator_node();
            operator_node->operator = t;
            operator_node->unary_tag = UNARY_EXPRESSION;
            push_stack(semantic_stack, &(void *) {operator_node});
            print_log_push(operator_node);
            break;
        }
        case ACTION59: {
            print_semantic_stack();
            pop_stack(semantic_stack, &popped);
            ASTNode *unary_expression;
            unary_expression = popped;
            print_log_pop(unary_expression);

            pop_stack(semantic_stack, &popped);
            ASTNode *ast_node;
            ast_node = popped;
            print_log_pop(ast_node);
            if (ast_node->tag == NODE_UNARY_OPERATOR &&
                (((UnaryOperatorNode *) ast_node)->unary_tag == UNARY_EXPRESSION)) {
                UnaryOperatorNode *sign_node = (UnaryOperatorNode *) ast_node;
                sign_node->operand = unary_expression;
                push_stack(semantic_stack, &(void *) {sign_node});
                print_log_push(sign_node);
            } else {
                push_stack(semantic_stack, &(void *) {ast_node});
                print_log_push(ast_node);
                push_stack(semantic_stack, &(void *) {unary_expression});
                print_log_push(ast_node);
            }
            print_semantic_stack();
            break;
        }
        case ACTION60: {
            Token *t = lexer->current_token;
            if (t->token_type != BO) {
                pop_stack(semantic_stack, &popped);
                UnaryOperatorNode *unary_operator_node;
                unary_operator_node = popped;
                unary_operator_node->unary_tag = UNARY_FACTOR;
                push_stack(semantic_stack, &(void *) {unary_operator_node});
            }
            break;
        }
        case ACTION61: {
            // push declare variable statements node
            DeclareVariableStatementNode *declare_variable_statement = create_declare_variable_statement_node();
            push_stack(semantic_stack, &(void *) {declare_variable_statement});
            break;
        }
        case ACTION62: {
            pop_stack(semantic_stack, &popped);
            ASTNode *ast_node = popped;
            if (ast_node->tag == NODE_DECLARE_VARIABLE_STATEMENT) {
                DeclareVariableStatementNode *declare_variable_statement = (DeclareVariableStatementNode *) ast_node;
                VariableNode *variable_node = create_variable_node();
                variable_node->id = create_id_node_with_token(lexer->current_token);
                add_element_arraylist
                        (declare_variable_statement->variable_nodes,
                         &variable_node);
                push_stack(semantic_stack, &(void *) {declare_variable_statement});
            } else {
                ModuleUseStatementNode *module_use_stmt_node = (ModuleUseStatementNode *) ast_node;
                module_use_stmt_node->has_assign_op_tag = true;
                IDNode *id_node = create_id_node();
                id_node->id_token = lexer->current_token;
                add_element_arraylist(module_use_stmt_node->result_id_list, &id_node);
                push_stack(semantic_stack, &(void *) {module_use_stmt_node});
                print_log_push(module_use_stmt_node);
            }
            break;
        }
        case ACTION63: {
            // set type of all declare statement variable nodes

            pop_stack(semantic_stack, &popped);
            DeclareVariableStatementNode *declare_statement_node;
            declare_statement_node = popped;
            ArrayList *variables_list =
                    declare_statement_node->variable_nodes;
            Token *type = lexer->current_token;
            // Set everyone's type:
            for (size_t i = 0; i < variables_list->size; ++i) {
                VariableNode *const *variable_node = get_element_at_index_arraylist(variables_list, i);
                (*variable_node)->type = type;
            }

            StatementsNode *const *statements_node = peek_stack(semantic_stack);
            add_element_arraylist((*statements_node)->statements_list, &declare_statement_node);
            break;
        }
        case ACTION64: {
            // push dummy array_variable node so that its range gets set automatically and utilize it later
            ArrayVariableNode *dummy_array_variable_node = create_array_variable_node();
            push_stack(semantic_stack, &(void *) {dummy_array_variable_node});
            break;
        }
        case ACTION65: {
            ArrayVariableNode *const *dummy_array_variable_node = peek_stack(semantic_stack);
            (*dummy_array_variable_node)->type = lexer->current_token;
            // set dummy's data type;
            break;
        }
        case ACTION66: {
            // dummy on top, iterate over variables list and create corresponding array variables

            pop_stack(semantic_stack, &popped);
            ArrayVariableNode *dummy_array_var_node;
            dummy_array_var_node = popped;
            Range dummy_range = dummy_array_var_node->array_range;
            const Token *dummy_type = dummy_array_var_node->type;


            pop_stack(semantic_stack, &popped);
            DeclareVariableStatementNode *declare_variable_statement;
            declare_variable_statement = popped;
            ArrayList *variable_nodes = declare_variable_statement->variable_nodes;

            DeclareArrayVariableStatementNode *declare_array_statement = create_declare_array_statement_node();
            ArrayList *array_variable_nodes =
                    declare_array_statement->array_variable_nodes;
            for (size_t i = 0; i < variable_nodes->size; ++i) {
                VariableNode *const *variable_node = get_element_at_index_arraylist(variable_nodes, i);
                ArrayVariableNode *array_var_node = create_array_variable_node();
                array_var_node->id = (*variable_node)->id;
                array_var_node->array_range = dummy_range;
                array_var_node->type = dummy_type;
                add_element_arraylist(array_variable_nodes, &array_var_node);
            }

            pop_stack(semantic_stack, &popped);
            StatementsNode *stmts_node;
            stmts_node = popped;
            add_element_arraylist(stmts_node->statements_list, &declare_array_statement);
            push_stack(semantic_stack, &(void *) {stmts_node});
            destroy_arraylist(variable_nodes);
            free(dummy_array_var_node);
            free(declare_variable_statement);
            break;
        }
        case ACTION67: {
            IDNode *id_node = create_id_node();
            id_node->id_token = lexer->current_token;
            push_stack(semantic_stack, &(void *) {id_node});
            print_log_push(id_node);
            break;
        }
        case ACTION68: {
            print_semantic_stack();
            pop_stack(semantic_stack, &popped);
            ASTNode *array_or_id_node;
            array_or_id_node = popped;
            print_log_pop(array_or_id_node);
            AssignmentStatementNode *assignment_statement = create_assignment_statement_node();
            assignment_statement->assign_line = get_current_token(lexer)->line_number;
            assignment_statement->lhs_node = array_or_id_node;
            push_stack(semantic_stack, &(void *) {assignment_statement});
            print_log_push(assignment_statement);
            print_semantic_stack();
            break;
        }
        case ACTION69: {
            print_semantic_stack();
            pop_stack(semantic_stack, &popped);
            ASTNode *expr_node;
            expr_node = popped;
            print_log_pop(expr_node);
            AssignmentStatementNode *const *assignment_node = peek_stack(semantic_stack);
            (*assignment_node)->rhs_node = expr_node;
            break;
        }
        case ACTION70: {
            // assignment complete
            pop_stack(semantic_stack, &popped);
            AssignmentStatementNode *assignment_statement;
            assignment_statement = popped;
            print_log_pop(assignment_statement);
            StatementsNode *const *statements_node = peek_stack(semantic_stack);
            add_element_arraylist((*statements_node)->statements_list, &assignment_statement);
            break;
        }
        case ACTION71: {
            pop_stack(semantic_stack, &popped);
            IDNode *id_node = popped;
            print_log_pop(id_node);
            ArrayNode *array_node = create_array_node();
            array_node->array_id = id_node;
            push_stack(semantic_stack, &array_node);
            print_log_push(array_node);
            print_semantic_stack();
            break;
        }
        case ACTION72: {
            WhileStatementNode *while_statement_node = create_while_statement_node();
            while_statement_node->start_line = get_current_token(lexer)->line_number;
            push_stack(semantic_stack, &(void *) {while_statement_node});
            print_log_push(while_statement_node);
            break;
        }
        case ACTION73: {
            pop_stack(semantic_stack, &popped);
            ASTNode *while_predicate = popped;
            print_log_pop(while_predicate);
            pop_stack(semantic_stack, &popped);
            WhileStatementNode *while_statement_node = popped;
            while_statement_node->predicate = while_predicate;
            push_stack(semantic_stack, &(void *) {while_statement_node});
            print_log_push(while_statement_node);
            break;
        }
        case ACTION74: {
            StatementsNode *while_statements_node = create_statements_node();
            push_stack(semantic_stack, &(void *) {while_statements_node});
            print_log_push(while_statements_node);
            break;
        }
        case ACTION75: {
            pop_stack(semantic_stack, &popped);
            StatementsNode *while_statements_node = popped;
            print_log_pop(while_statements_node);
            pop_stack(semantic_stack, &popped);
            WhileStatementNode *while_statement_node = popped;
            while_statement_node->statements_node = while_statements_node;
            while_statement_node->end_line = get_current_token(lexer)->line_number;
            print_log_pop(while_statement_node);
            pop_stack(semantic_stack, &popped);
            StatementsNode *enclosing_statements = popped;
            print_log_pop(enclosing_statements);
            ASTNode *while_ast = (ASTNode *) while_statement_node;
            add_element_arraylist(enclosing_statements->statements_list, &while_ast);
            push_stack(semantic_stack, &(void *) {enclosing_statements});
            print_log_push(enclosing_statements);
            break;
        }
        case ACTION76: {
            ModuleUseStatementNode *module_use_statement_node = create_module_use_statement_node();
            push_stack(semantic_stack, &(void *) {module_use_statement_node});
            print_log_push(module_use_statement_node);
            break;
        }
        case ACTION77: {
            pop_stack(semantic_stack, &popped);
            ModuleUseStatementNode *module_use_stmt_node;
            module_use_stmt_node = popped;
            print_log_pop(module_use_stmt_node);
            module_use_stmt_node->module_name = create_id_node_with_token(lexer->current_token);
            push_stack(semantic_stack, &(void *) {module_use_stmt_node});
            print_log_push(module_use_stmt_node);
            break;
        }
        case ACTION78: {
            Token *t = lexer->current_token;
            if (t->token_type != PLUS && t->token_type != MINUS) {
                return;
            }
            UnaryOperatorNode *operator_node = create_unary_operator_node();
            operator_node->operator = t;
            operator_node->unary_tag = UNARY_FACTOR;
            push_stack(semantic_stack, &(void *) {operator_node});
            print_log_push(operator_node);
            break;
        }
        case ACTION79: {
            IntegerLiteralNode *integer_literal_factor = create_integer_literal_node();
            integer_literal_factor->num = lexer->current_token;

            pop_stack(semantic_stack, &popped);
            ASTNode *ast_node = popped;
            if (ast_node->tag == NODE_UNARY_OPERATOR &&
                ((UnaryOperatorNode *) ast_node)->unary_tag == UNARY_FACTOR) {
                UnaryOperatorNode *unary_op_node = (UnaryOperatorNode *) ast_node;
                unary_op_node->operand = (ASTNode *) integer_literal_factor;
                push_stack(semantic_stack, &(void *) {unary_op_node});
            } else {
                push_stack(semantic_stack, &(void *) {ast_node});
                push_stack(semantic_stack, &(void *) {integer_literal_factor});
                print_log_push(integer_literal_factor);
            }
            pop_stack(semantic_stack, &popped);
            ast_node = popped;
            print_log_pop(ast_node);

            ModuleUseStatementNode *module_use_stmt_node;
            pop_stack(semantic_stack, &popped);
            module_use_stmt_node = popped;
            print_log_pop(module_use_stmt_node);

            add_element_arraylist(module_use_stmt_node->parameter_list, &ast_node);
            push_stack(semantic_stack, &(void *) {module_use_stmt_node});
            print_log_push(module_use_stmt_node);
            break;
        }
        case ACTION80: {
            RealLiteralNode *real_literal_factor = create_real_literal_node();
            real_literal_factor->r_num = lexer->current_token;

            pop_stack(semantic_stack, &popped);
            ASTNode *ast_node = popped;
            if (ast_node->tag == NODE_UNARY_OPERATOR &&
                ((UnaryOperatorNode *) ast_node)->unary_tag == UNARY_FACTOR) {
                UnaryOperatorNode *unary_op_node = (UnaryOperatorNode *) ast_node;
                unary_op_node->operand = (ASTNode *) real_literal_factor;
                push_stack(semantic_stack, &(void *) {unary_op_node});
            } else {
                push_stack(semantic_stack, &(void *) {ast_node});
                push_stack(semantic_stack, &(void *) {real_literal_factor});
                print_log_push(real_literal_factor);
            }
            pop_stack(semantic_stack, &popped);
            ast_node = popped;
            print_log_pop(ast_node);

            ModuleUseStatementNode *module_use_stmt_node;
            pop_stack(semantic_stack, &popped);
            module_use_stmt_node = popped;
            print_log_pop(module_use_stmt_node);

            add_element_arraylist(module_use_stmt_node->parameter_list, &ast_node);
            push_stack(semantic_stack, &(void *) {module_use_stmt_node});
            print_log_push(module_use_stmt_node);
            break;
        }
        case ACTION81: {
            //  print_semantic_stack();
            IDNode *id_node = create_id_node();
            id_node->id_token = lexer->current_token;
            pop_stack(semantic_stack, &popped);
            ASTNode *ast_node = popped;
            if (ast_node->tag == NODE_UNARY_OPERATOR &&
                ((UnaryOperatorNode *) ast_node)->unary_tag == UNARY_FACTOR) {
                UnaryOperatorNode *unary_operator_node = (UnaryOperatorNode *) ast_node;
                unary_operator_node->operand = (ASTNode *) id_node;
                push_stack(semantic_stack, &(void *) {unary_operator_node});
            } else {
                push_stack(semantic_stack, &(void *) {ast_node});
                push_stack(semantic_stack, &(void *) {id_node});
                print_log_push(id_node);
            }

            break;
            // print_semantic_stack();
        }
        case ACTION82: {
            print_semantic_stack();
            pop_stack(semantic_stack, &popped);
            ModuleUseStatementNode *module_use_stmt_node;
            module_use_stmt_node = popped;
            print_log_pop(module_use_stmt_node);

            pop_stack(semantic_stack, &popped);
            StatementsNode *stmts_node = popped;
            print_log_pop(stmts_node);
            add_element_arraylist(stmts_node->statements_list, &module_use_stmt_node);
            push_stack(semantic_stack, &(void *) {stmts_node});
            print_log_push(stmts_node);
            break;
        }
        case ACTION83: {
            pop_stack(semantic_stack, &popped);
            ASTNode *ast_node = popped;
            print_log_pop(ast_node);

            ModuleUseStatementNode *module_use_stmt_node;
            pop_stack(semantic_stack, &popped);
            module_use_stmt_node = popped;
            print_log_pop(module_use_stmt_node);

            add_element_arraylist(module_use_stmt_node->parameter_list, &ast_node);
            push_stack(semantic_stack, &(void *) {module_use_stmt_node});
            print_log_push(module_use_stmt_node);
            break;
        }
        case ACTION84: {
            print_semantic_stack();
            if (lexer->current_token->token_type == NUM) {
                IntegerLiteralNode *integer_literal_node = create_integer_literal_node();
                integer_literal_node->num = lexer->current_token;
                pop_stack(semantic_stack, &popped);
                ASTNode *top_node = popped;
                if (top_node->tag == NODE_UNARY_OPERATOR &&
                    ((UnaryOperatorNode *) top_node)->unary_tag == UNARY_FACTOR) {
                    UnaryOperatorNode *unary_operator_node = (UnaryOperatorNode *) top_node;
                    unary_operator_node->operand = (ASTNode *) integer_literal_node;
                    push_stack(semantic_stack, &(void *) {unary_operator_node});
                } else {
                    push_stack(semantic_stack, &(void *) {top_node});
                    push_stack(semantic_stack, &(void *) {integer_literal_node});
                    print_log_push(integer_literal_node);
                }
            } else {
                IDNode *id_node = create_id_node();
                id_node->id_token = lexer->current_token;
                pop_stack(semantic_stack, &popped);
                ASTNode *top_node = popped;
                print_log_pop(top_node);
                if (top_node->tag == NODE_UNARY_OPERATOR &&
                    ((UnaryOperatorNode *) top_node)->unary_tag == UNARY_FACTOR) {
                    UnaryOperatorNode *unary_operator_node = (UnaryOperatorNode *) top_node;
                    unary_operator_node->operand = (ASTNode *) id_node;
                    push_stack(semantic_stack, &(void *) {unary_operator_node});
                } else {
                    push_stack(semantic_stack, &(void *) {top_node});
                    push_stack(semantic_stack, &(void *) {id_node});
                    print_log_push(id_node);
                }
            }
            print_semantic_stack();
            break;
        }

        case ACTION85: {
            pop_stack(semantic_stack, &popped);
            ASTNode *ast_node = popped;
            print_log_pop(ast_node);

            pop_stack(semantic_stack, &popped);
            PrintStatementNode *print_statement_node = popped;
            print_log_pop(print_statement_node);
            print_statement_node->print_item = ast_node;

            // Now print_statement is complete, driverStatements node must be at the top
            pop_stack(semantic_stack, &popped);
            StatementsNode *driver_statements_node = popped;
            print_log_pop(driver_statements_node);

            // Add print element to the list of driver statements;
            add_element_arraylist(driver_statements_node->statements_list, &print_statement_node);

            // Push driver statements node back
            push_stack(semantic_stack, &(void *) {driver_statements_node});
            print_log_push(driver_statements_node);
            break;
        }
        default:
            break;
    }
}




