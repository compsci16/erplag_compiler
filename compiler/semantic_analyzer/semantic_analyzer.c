#include "semantic_analyzer.h"
#include "ast.h"
#include "compiler/error_reporter/error_report.h"
#include "utility.h"
#include "node_utility.h"

bool GOT_SEMANTIC_ERROR_GLOBAL = false;

#define PERFORM_SEMANTIC_ANALYSIS_ON(node) perform_semantic_analysis_helper((ASTNode *) (node))
typedef enum Where {
    IN_PARAMETER,
    IN_RETURN
} Where;

typedef enum ManyOrFew {
    TOO_FEW,
    TOO_MANY,
    EQUAL
} ManyOrFew;

typedef enum FunctionCallType {
    NORMAL_CALL,
    RECURSIVE_CALL,
    UNDEFINED_AND_UNDECLARED_CALL,
    UNDEFINED_AND_DECLARED_CALL,
    DEFINED_LATER_NO_DECLARATION_CALL,
    DEFINED_AND_DECLARED_BEFORE_CALL
} FunctionCallType;

static void report_unassigned_return_parameter(int parameter_number, char *parameter_name, int start_line, int end_line,
                                               char *module_name);

static void report_assigned_for_variable(char *parameter_name, int start_line,
                                         int end_line);

static void report_out_of_bounds_error(char *identifier, int line_number);

static void report_assignment_error(int assign_line, TypeDescriptor lhs_type_descriptor,
                                    TypeDescriptor rhs_type_descriptor);

static FunctionCallType handle_function_call(ModuleUseStatementNode *module_use_statement_node);

static void report_wrong_number_in_call(int call_line, char *module_name, int defined_line, size_t expected,
                                        size_t actual, Where where, ManyOrFew many_or_few);

static void
handle_return_list(ArrayList *parameter_list, TypeDescriptor *caller_return_type_list,
                   TypeDescriptor *callee_return_type_list, size_t num_returns, char *module_name,
                   int call_line, int defined_line);

static FunctionCallType get_function_call_type(ModuleUseStatementNode *module_use_statement_node);

static void report_different_descriptor(int call_line, char *module_name, int defined_line, int param_number,
                                        TypeDescriptor caller_descriptor, TypeDescriptor callee_descriptor,
                                        Where where);


static void perform_semantic_analysis_helper(ASTNode *node);

static void report_switch_type_error(int switch_line);

static void report_unnecessary_default(int start_line, int end_line);

static void report_missing_default(int start_line, int end_line);

static void
report_required_type_error(int line_number, TypeDescriptor required_type_descriptor,
                           TypeDescriptor actual_type_descriptor);

static void
handle_parameter_list(ArrayList *parameter_list, TypeDescriptor *caller_parameter_type_list,
                      TypeDescriptor *callee_parameter_type_list, size_t num_parameters, char *module_name,
                      int call_line, int defined_line);

static void report_undeclared_identifier_in_call(char *identifier, int call_line, int param_number);

static void
report_invalid_binary_operands(TypeDescriptor lhs_descriptor, TypeDescriptor rhs_descriptor, int operator_token_type,
                               int line_number);

// to avoid passing it again and again during recursion
static SymbolTable *global_symbol_table = NULL;

void perform_semantic_analysis(ProgramNode *root, SymbolTable *global_symbol_table_parameter) {
    global_symbol_table = global_symbol_table_parameter;
    PERFORM_SEMANTIC_ANALYSIS_ON(root);
}

static void perform_semantic_analysis_helper(ASTNode *node) {
    switch (node->tag) {
        case NODE_ARRAY_VARIABLE:
            break;
        case NODE_BOOLEAN_LITERAL: {
            BooleanLiteralNode *boolean_literal_node = (BooleanLiteralNode *) node;
            boolean_literal_node->boolean_literal_type_descriptor = (TypeDescriptor) {.form = TYPE_BOOLEAN};
            break;
        }
        case NODE_CASE: {
            // Why not call here? Better to do in switch as already accessing all cases via the list here
            CaseStatementNode *case_statement_node = (CaseStatementNode *) node;
            PERFORM_SEMANTIC_ANALYSIS_ON(case_statement_node->statements_node);
            break;
        }
        case NODE_GET_VALUE:
            break;
        case NODE_INTEGER_LITERAL: {
            IntegerLiteralNode *integer_literal_node = (IntegerLiteralNode *) node;
            integer_literal_node->integer_literal_type_descriptor = (TypeDescriptor) {.form = TYPE_INTEGER};
            break;
        }
        case NODE_MODULE_DECLARATIONS:
            break;
        case NODE_MODULE: {
            ModuleNode *module_node = (ModuleNode *) node;
            StatementsNode *module_statements_node = module_node->statements_node;
            PERFORM_SEMANTIC_ANALYSIS_ON(module_statements_node);
            SymbolTableEntry *symbol_table_entry = module_node->symbol_table_entry;
            int defined_line = symbol_table_entry->line_number;
            int start_line = module_node->start_line;
            int end_line = module_node->end_line;
            ArrayList *return_list = module_node->output_parameter_list_node->variable_nodes_list;
            char *module_name = module_node->module_id->id_token->lexeme;
            for (size_t i = 0; i < return_list->size; ++i) {
                VariableNode **p_var_node = get_element_at_index_arraylist(return_list, i);
                VariableNode *variable_node = *p_var_node;
                IDNode *corresponding_id = variable_node->id;
                SymbolTableEntry *return_parameter_entry =
                        search_in_cactus_stack(module_node->function_entry_symbol_table->children[0],
                                               corresponding_id->id_token->lexeme);
                if (!return_parameter_entry->was_assigned) {
                    char *parameter_name = corresponding_id->id_token->lexeme;
                    report_unassigned_return_parameter((int) i + 1, parameter_name, start_line,
                                                       end_line, module_name);
                }
            }
            break;
        }
        case NODE_MODULE_LIST:
            break;
        case NODE_PRINT_STATEMENT:
            break;
        case NODE_PROGRAM: {
            ProgramNode *program_node = (ProgramNode *) node;
            PERFORM_SEMANTIC_ANALYSIS_ON(program_node->driver_module);
            size_t num_modules = program_node->modules_list_node->modules_list->size;
            ArrayList *modules_list = program_node->modules_list_node->modules_list;
            for (size_t i = 0; i < num_modules; ++i) {
                ModuleNode **module_node = get_element_at_index_arraylist(modules_list, i);
                PERFORM_SEMANTIC_ANALYSIS_ON(*module_node);
            }
            break;
        }
        case NODE_REAL_LITERAL: {
            RealLiteralNode *real_literal_node = (RealLiteralNode *) node;
            real_literal_node->real_literal_type_descriptor = (TypeDescriptor) {.form =  TYPE_REAL};
            break;
        }
        case NODE_STATEMENTS: {
            StatementsNode *statements_node = (StatementsNode *) node;
            ArrayList *statements_list = statements_node->statements_list;
            size_t num_statements = statements_list->size;
            for (int i = 0; i < num_statements; ++i) {
                ASTNode **p_ast_node = get_element_at_index_arraylist(statements_list, i);
                ASTNode *ast_node = *p_ast_node;
                NODE_TYPE_TAG tag = ast_node->tag;
                if (tag == NODE_MODULE_USE_STATEMENT ||
                    tag == NODE_WHILE_STATEMENT ||
                    tag == NODE_FOR ||
                    tag == NODE_SWITCH ||
                    tag == NODE_CASE ||
                    tag == NODE_ASSIGNMENT_STATEMENT) {
                    PERFORM_SEMANTIC_ANALYSIS_ON(ast_node);
                }
            }
            break;
        }
        case NODE_SWITCH: {
            SwitchStatementNode *switch_statement_node = (SwitchStatementNode *) node;
            TypeDescriptor switch_id_descriptor = switch_statement_node->id->identifier_type_descriptor;
            int start_line = switch_statement_node->id->id_token->line_number;
            int end_line = switch_statement_node->end_line;
            TypeForm switch_form = switch_id_descriptor.form;
            bool need_to_compare_case_values = switch_form == TYPE_INTEGER || switch_form == TYPE_BOOLEAN;
            if (switch_form != TYPE_UNDEFINED && !need_to_compare_case_values) {
                report_switch_type_error(start_line);
                break;
            }

            ArrayList *case_list = switch_statement_node->case_list;
            for (size_t i = 0; i < case_list->size; ++i) {
                ASTNode **p_ast_node = get_element_at_index_arraylist(case_list, i);
                CaseStatementNode *case_statement_node = (CaseStatementNode *) (*p_ast_node);
                int case_line = case_statement_node->case_value->line_number;
                if (need_to_compare_case_values) {
                    TypeDescriptor case_type_descriptor = case_statement_node->type_descriptor;
                    if (!are_equivalent_type_descriptors(case_type_descriptor, switch_id_descriptor)) {
                        report_required_type_error(case_line, switch_id_descriptor, case_type_descriptor);
                    }
                }
                PERFORM_SEMANTIC_ANALYSIS_ON(case_statement_node);
            }
            CaseStatementNode *default_statements_node = switch_statement_node->default_case_statement_node;
            if (switch_id_descriptor.form != TYPE_BOOLEAN && default_statements_node == NULL) {
                report_missing_default(start_line, end_line);
            } else if (switch_form == TYPE_BOOLEAN && default_statements_node != NULL) {
                report_unnecessary_default(start_line, end_line);
            }
            if (default_statements_node != NULL) PERFORM_SEMANTIC_ANALYSIS_ON(default_statements_node);
            break;
        }
        case NODE_VARIABLE_LIST:
            break;
        case NODE_VARIABLE:
            break;
        case NODE_FOR: {
            ForNode *for_node = (ForNode *) node;
            PERFORM_SEMANTIC_ANALYSIS_ON(for_node->statements_node);
            IDNode *id_node = for_node->variable_node->id;
            SymbolTableEntry *loop_variable_entry = search_in_symbol_table(for_node->for_symbol_table,
                                                                           id_node->id_token->lexeme);
            if (loop_variable_entry->was_assigned) {
                char *parameter_name = id_node->id_token->lexeme;
                int start_line = for_node->for_symbol_table->start_line;
                int end_line = for_node->for_symbol_table->end_line;
                report_assigned_for_variable(parameter_name, start_line, end_line);
            }
            break;
        }
        case NODE_ARRAY: {
            ArrayNode *array_node = (ArrayNode *) node;
            IDNode *id_node = array_node->array_id;
            if (id_node->identifier_type_descriptor.form == TYPE_UNDEFINED) {
                array_node->array_type_descriptor.form = TYPE_UNDEFINED;
                break;
            }
            array_node->array_type_descriptor.form = (TypeForm) id_node->identifier_type_descriptor.array_type.atomic_type;
            PERFORM_SEMANTIC_ANALYSIS_ON(array_node->array_expression);
            if (array_node->array_expression->tag != NODE_INTEGER_LITERAL) break;
            IntegerLiteralNode *array_index = (IntegerLiteralNode *) array_node->array_expression;
            Range array_range = id_node->identifier_type_descriptor.array_type.array_range;
            if (array_range.begin->token_type == NUM &&
                array_range.end->token_type == NUM) {
                int low = atoi(array_range.begin->lexeme);
                int high = atoi(array_range.end->lexeme);
                if (array_range.begin_sign->token_type == MINUS) {
                    low = (-1) * low;
                }
                if (array_range.end_sign->token_type == MINUS) {
                    high = (-1) * high;
                }
                int current_id = atoi(array_index->num->lexeme);
                if (current_id < low || current_id > high) {
                    id_node->identifier_type_descriptor = (TypeDescriptor) {.form = TYPE_UNDEFINED};
                    Token *id_token = id_node->id_token;
                    report_out_of_bounds_error(id_token->lexeme, id_token->line_number);
                }
            }

            break;
        }
        case NODE_BINARY_OPERATOR: {
            BinaryOperatorNode *binary_operator_node = (BinaryOperatorNode *) node;
            PERFORM_SEMANTIC_ANALYSIS_ON(binary_operator_node->left);
            PERFORM_SEMANTIC_ANALYSIS_ON(binary_operator_node->right);
            int line_number = binary_operator_node->operator->line_number;
            TypeDescriptor lhs_type_descriptor = get_type_descriptor(binary_operator_node->left);
            TypeDescriptor rhs_type_descriptor = get_type_descriptor(binary_operator_node->right);
            TypeForm lhs_form = lhs_type_descriptor.form;
            TypeForm rhs_form = rhs_type_descriptor.form;
            TypeDescriptor error_type_descriptor = {.form = TYPE_ERROR};
            if (lhs_type_descriptor.form == TYPE_ERROR || rhs_type_descriptor.form == TYPE_ERROR) {
                binary_operator_node->binary_operator_type_descriptor = error_type_descriptor;
                return;
            }
            if (lhs_type_descriptor.form == TYPE_UNDEFINED || rhs_type_descriptor.form == TYPE_UNDEFINED) {
                binary_operator_node->binary_operator_type_descriptor = error_type_descriptor;
                return;
            }
            Token *operator_token = binary_operator_node->operator;
            int op_token_type = operator_token->token_type;
            switch (operator_token->token_type) {
                case PLUS:
                case MINUS:
                case MUL: {
                    if (lhs_form == TYPE_INTEGER && rhs_form == TYPE_INTEGER) {
                        binary_operator_node->binary_operator_type_descriptor = lhs_type_descriptor;
                        return;
                    }
                    if (lhs_form == TYPE_REAL && rhs_form == TYPE_REAL) {
                        binary_operator_node->binary_operator_type_descriptor = lhs_type_descriptor;
                        return;
                    }
                    binary_operator_node->binary_operator_type_descriptor = error_type_descriptor;
                    report_invalid_binary_operands(lhs_type_descriptor, rhs_type_descriptor, op_token_type,
                                                   line_number);
                    return;
                }
                case DIV: {
                    if ((lhs_form == TYPE_INTEGER && rhs_form == TYPE_INTEGER)
                        || (lhs_form == TYPE_REAL && rhs_form == TYPE_REAL)
                        || (lhs_form == TYPE_INTEGER && rhs_form == TYPE_REAL)
                        || (lhs_form == TYPE_REAL && rhs_form == TYPE_INTEGER)) {
                        binary_operator_node->binary_operator_type_descriptor = (TypeDescriptor) {.form = TYPE_REAL};
                        return;
                    }
                    binary_operator_node->binary_operator_type_descriptor = error_type_descriptor;
                    report_invalid_binary_operands(lhs_type_descriptor, rhs_type_descriptor, op_token_type,
                                                   line_number);
                    return;
                }
                    /* relational operator */
                case LT:
                case GT:
                case LE:
                case GE:
                case EQ:
                case NE: {
                    if ((lhs_form == TYPE_INTEGER && rhs_form == TYPE_INTEGER)
                        || (lhs_form == TYPE_REAL && rhs_form == TYPE_REAL)) {
                        binary_operator_node->binary_operator_type_descriptor = (TypeDescriptor) {.form = TYPE_BOOLEAN};
                        return;
                    }
                    binary_operator_node->binary_operator_type_descriptor = error_type_descriptor;
                    report_invalid_binary_operands(lhs_type_descriptor, rhs_type_descriptor, op_token_type,
                                                   line_number);
                    return;
                }
                    /* logical */
                case AND:
                case OR: {
                    if (lhs_form == TYPE_BOOLEAN && rhs_form == TYPE_BOOLEAN) {
                        binary_operator_node->binary_operator_type_descriptor = (TypeDescriptor) {.form = TYPE_BOOLEAN};
                        return;
                    }
                    binary_operator_node->binary_operator_type_descriptor = error_type_descriptor;
                    report_invalid_binary_operands(lhs_type_descriptor, rhs_type_descriptor, op_token_type,
                                                   line_number);
                    return;
                }
            }

            break;
        }
        case NODE_UNARY_OPERATOR: {
            UnaryOperatorNode *unary_operator_node = (UnaryOperatorNode *) node;
            PERFORM_SEMANTIC_ANALYSIS_ON(unary_operator_node->operand);
            unary_operator_node->unary_operator_type_descriptor = get_type_descriptor(unary_operator_node->operand);
            break;
        }
        case NODE_ID: {
            break;
        }
        case NODE_DECLARE_VARIABLE_STATEMENT:
            break;
        case NODE_DECLARE_ARRAY_VARIABLE_STATEMENT:
            break;
        case NODE_ASSIGNMENT_STATEMENT: {
            AssignmentStatementNode *assignment_statement_node = (AssignmentStatementNode *) node;
            // TODO: check if LHS assignable first
            ASTNode *lhs_node = assignment_statement_node->lhs_node;
            ASTNode *rhs_node = assignment_statement_node->rhs_node;
            PERFORM_SEMANTIC_ANALYSIS_ON(lhs_node);
            PERFORM_SEMANTIC_ANALYSIS_ON(rhs_node);
            TYPE_DESCRIPTOR_INIT(lhs_type_descriptor);
            TYPE_DESCRIPTOR_INIT(rhs_type_descriptor);
            lhs_type_descriptor = get_type_descriptor(lhs_node);
            rhs_type_descriptor = get_type_descriptor(rhs_node);
            if (lhs_type_descriptor.form == TYPE_UNDEFINED || rhs_type_descriptor.form == TYPE_UNDEFINED) {
                break;
            }
            int assign_line = assignment_statement_node->assign_line;
            if (rhs_type_descriptor.form != TYPE_ERROR) {
                if (!are_equivalent_type_descriptors(lhs_type_descriptor, rhs_type_descriptor)) {
                    report_assignment_error(assign_line, lhs_type_descriptor, rhs_type_descriptor);
                }
            }
            break;
        }
        case NODE_WHILE_STATEMENT: {
            WhileStatementNode *while_statement_node = (WhileStatementNode *) node;
            PERFORM_SEMANTIC_ANALYSIS_ON(while_statement_node->statements_node);
            break;
        }
        case NODE_MODULE_USE_STATEMENT: {
            ModuleUseStatementNode *module_use_statement_node = (ModuleUseStatementNode *) node;
            int call_line = module_use_statement_node->module_name->id_token->line_number;

            FunctionCallType function_call_type = handle_function_call(module_use_statement_node);

            if (function_call_type != NORMAL_CALL) break;

            char *module_name = module_use_statement_node->module_name->id_token->lexeme;
            SymbolTableEntry *module_entry = search_in_symbol_table(global_symbol_table, module_name);
            TypeDescriptor callee_type_descriptor = module_entry->type_descriptor;
            ArrayList *parameter_list = module_use_statement_node->parameter_list;
            int defined_line = module_entry->line_number;
            size_t found_num_parameters = parameter_list->size;
            size_t expected_num_parameters = callee_type_descriptor.function_type.num_parameters;
            TypeDescriptor caller_type_descriptor = module_use_statement_node->caller_type_descriptor;
            if (expected_num_parameters != found_num_parameters) {
                report_wrong_number_in_call(call_line, module_name, defined_line, expected_num_parameters,
                                            found_num_parameters, IN_PARAMETER,
                                            expected_num_parameters > found_num_parameters ? TOO_FEW : TOO_MANY);
            } else {
                handle_parameter_list(parameter_list, caller_type_descriptor.function_type.parameter_types,
                                      callee_type_descriptor.function_type.parameter_types,
                                      expected_num_parameters, module_name, call_line, defined_line);
            }
            // Right Number of Parameters, now get TypeDescriptor and check that for equivalence
            size_t expected_num_returns = callee_type_descriptor.function_type.num_returns;
            size_t actual_num_returns = caller_type_descriptor.function_type.num_returns;
            if (expected_num_returns != actual_num_returns) {
                report_wrong_number_in_call(call_line, module_name, defined_line, expected_num_returns,
                                            actual_num_returns, IN_RETURN,
                                            expected_num_parameters > found_num_parameters ? TOO_FEW : TOO_MANY);
            }
                // Compare return type descriptors for a,b in:  [a,b] := use module mod1 with x;
            else {
                handle_return_list(module_use_statement_node->result_id_list,
                                   caller_type_descriptor.function_type.return_types,
                                   callee_type_descriptor.function_type.return_types,
                                   expected_num_returns, module_name, call_line, defined_line);
            }
            break;
        }
        case FINAL_ENTRY_COUNT_OF_NODE_TYPES:
            break;
    }
}

static void
report_invalid_binary_operands(TypeDescriptor lhs_descriptor, TypeDescriptor rhs_descriptor, int operator_token_type,
                               int line_number) {
    char error[256];
    char *lhs_desc = type_descriptor_to_string(lhs_descriptor);
    char *rhs_desc = type_descriptor_to_string(rhs_descriptor);
    snprintf(error, sizeof error,
             "Semantic Error on Line #%d: Invalid operands to binary operator %s ('%s' and '%s')\n",
             line_number, SYMBOL_TO_STRING[operator_token_type], lhs_desc, rhs_desc);
    free(lhs_desc);
    free(rhs_desc);
    report_error(SEMANTIC_ERROR, error);
}

static void report_unassigned_return_parameter(int parameter_number, char *parameter_name, int start_line, int end_line,
                                               char *module_name) {
    char error[256];
    snprintf(error, sizeof error,
             "Semantic Error for function scope Line %d to Line %d, Parameter #%d: '%s' is never assigned a value within the module '%s'\n",
             start_line, end_line, parameter_number, parameter_name, module_name);
    report_error(SEMANTIC_ERROR, error);
}

static void report_assigned_for_variable(char *parameter_name, int start_line,
                                         int end_line) {
    char error[256];
    snprintf(error, sizeof error,
             "Semantic Error, For loop variable: '%s' is assigned a value within the for scope lines %d-%d\n",
             parameter_name, start_line, end_line);
    report_error(SEMANTIC_ERROR, error);
}

static void report_assignment_error(int assign_line, TypeDescriptor lhs_type_descriptor,
                                    TypeDescriptor rhs_type_descriptor) {
    char error[256];
    snprintf(error, sizeof error,
             "Semantic Error on Line #%d: Assigning to '%s' from incompatible type '%s'\n",
             assign_line,
             type_descriptor_to_string(lhs_type_descriptor),
             type_descriptor_to_string(rhs_type_descriptor));
    report_error(SEMANTIC_ERROR, error);
}

static void report_switch_type_error(int switch_line) {
    char error[256];
    snprintf(error, sizeof error,
             "Semantic Error on Line #%d: switch requires identifier of type integer or boolean.\n",
             switch_line);
    report_error(SEMANTIC_ERROR, error);
}

static void report_unnecessary_default(int start_line, int end_line) {
    char error[256];
    snprintf(error, sizeof error,
             "Semantic Error on switch scope Lines %d-%d: switch with type boolean identifier should not have a default.\n",
             start_line, end_line);
    report_error(SEMANTIC_ERROR, error);
}

static FunctionCallType handle_function_call(ModuleUseStatementNode *module_use_statement_node) {
    char *name = module_use_statement_node->module_name->id_token->lexeme;
    FunctionCallType function_call_type = get_function_call_type(module_use_statement_node);
    switch (function_call_type) {
        case NORMAL_CALL:
            break;
        case RECURSIVE_CALL: {
            char error_message[256];
            snprintf(error_message, sizeof error_message,
                     "Semantic Error on Line #%d: Function '%s' is within a recursive call chain.\n",
                     module_use_statement_node->module_name->id_token->line_number, name);
            report_error(SEMANTIC_ERROR, error_message);
            break;
        }
        case UNDEFINED_AND_UNDECLARED_CALL: {
            char error_message[256];
            snprintf(error_message, sizeof error_message,
                     "Semantic Error on Line #%d: Use of unknown module %s.\n",
                     module_use_statement_node->module_name->id_token->line_number, name);
            report_error(SEMANTIC_ERROR, error_message);
            break;
        }
        case UNDEFINED_AND_DECLARED_CALL: {
            char error_message[256];
            snprintf(error_message, sizeof error_message,
                     "Semantic Error on Line #%d: Module '%s' is declared but never defined.\n",
                     module_use_statement_node->module_name->id_token->line_number, name);
            report_error(SEMANTIC_ERROR, error_message);
            break;
        }
        case DEFINED_LATER_NO_DECLARATION_CALL: {
            char error_message[256];
            snprintf(error_message, sizeof error_message,
                     "Semantic Error on Line #%d: Module '%s' must be defined or declared before it's invocation.\n",
                     module_use_statement_node->module_name->id_token->line_number, name);
            report_error(SEMANTIC_ERROR, error_message);
            break;
        }
        case DEFINED_AND_DECLARED_BEFORE_CALL: {
            char error_message[256];
            snprintf(error_message, sizeof error_message,
                     "Semantic Error on Line #%d: Module '%s' is both defined and declared before it's invocation.\n",
                     module_use_statement_node->module_name->id_token->line_number, name);
            report_error(SEMANTIC_ERROR, error_message);
            break;
            break;
        }
    }
    return function_call_type;
}

static void report_missing_default(int start_line, int end_line) {
    char error_message[256];
    snprintf(error_message, sizeof error_message,
             "Semantic Error for switch scope Lines %d-%d: Potential uncovered code path; add a default label.\n",
             start_line, end_line);
    report_error(SEMANTIC_ERROR, error_message);
}

static FunctionCallType get_function_call_type(ModuleUseStatementNode *module_use_statement_node) {
    char *callee_name = module_use_statement_node->module_name->id_token->lexeme;
    /*
     * Traverse upto the symbol table that is a child of global symbol table,
     * now this must be the symbol table of some module, say X.
     * Now X's entry in global symbol table entry contains a child symbol table.
     * We need to ensure that X.function_type.symbol_table != the symbol table found by traversing upwards
    */
    SymbolTable *current = module_use_statement_node->parent_symbol_table;
    while (current->parent != global_symbol_table) {
        current = current->parent;
    }
    SymbolTable *module_dec_symbol_table = global_symbol_table->children[0];
    SymbolTableEntry *callees_symbol_table_entry_in_global = search_in_symbol_table(global_symbol_table, callee_name);
    SymbolTableEntry *declared_entry = search_in_symbol_table(module_dec_symbol_table, callee_name);

    if (callees_symbol_table_entry_in_global == NULL) {
        // Undefined function!
        if (declared_entry == NULL) {
            return UNDEFINED_AND_UNDECLARED_CALL;
        }
        return UNDEFINED_AND_DECLARED_CALL;
    }

    if (callees_symbol_table_entry_in_global->type_descriptor.function_type.module_symbol_table == current) {
        return RECURSIVE_CALL;
    }
    int caller_line_number = module_use_statement_node->module_name->id_token->line_number;
    int callee_definition_line_number = callees_symbol_table_entry_in_global->line_number;
    if (callee_definition_line_number > caller_line_number) {
        // Use of callee before defining it, check if it was declared at least
        if (declared_entry == NULL) {
            // defined but defined after current line and has no declaration before
            return DEFINED_LATER_NO_DECLARATION_CALL;
        }
        // declarations come at top in syntactically correct code, so if it was declared it must be before current line
        return NORMAL_CALL;
    } else {
        if (declared_entry != NULL) return DEFINED_AND_DECLARED_BEFORE_CALL;
    }
    return NORMAL_CALL;
}

static void report_required_type_error(int line_number, TypeDescriptor required_type_descriptor,
                                       TypeDescriptor actual_type_descriptor) {
    char error_message[256];
    snprintf(error_message, sizeof error_message, "Semantic Error on Line #%d: Required Type: '%s', Provided: '%s'\n",
             line_number,
             type_descriptor_to_string(required_type_descriptor),
             type_descriptor_to_string(actual_type_descriptor));
    report_error(SEMANTIC_ERROR, error_message);
}

/*
 * Check's the type descriptor of caller against the type descriptor of callee's parameters list
 * Reports and sets UNDEFINED entry in symbol table for
 * previously undeclaredvariables in caller's parameter list.
*/
static void
handle_parameter_list(ArrayList *parameter_list, TypeDescriptor *caller_parameter_type_list,
                      TypeDescriptor *callee_parameter_type_list,
                      size_t num_parameters,
                      char *module_name, int call_line, int defined_line) {
    // Remember that both ArrayVar and Var Nodes can be in VariableListNode
    // Prepare caller's descriptor from the parameter list; report undeclared error if necessary (i.e. not found in ST)
    for (size_t i = 0; i < num_parameters; ++i) {
        TypeDescriptor caller_i = caller_parameter_type_list[i];
        TypeDescriptor callee_i = callee_parameter_type_list[i];
        if (caller_i.form == TYPE_UNDEFINED) {
            ASTNode **parameter = get_element_at_index_arraylist(parameter_list, i);
            char *identifier = GET_IDENTIFIER_FROM(*parameter);
            report_undeclared_identifier_in_call(identifier, call_line, (int) i + 1);
            continue;
        }

        if (!are_equivalent_type_descriptors(caller_i, callee_i)) {
            report_different_descriptor(call_line, module_name, defined_line, (int) i + 1,
                                        caller_i, callee_i, IN_PARAMETER);
        }
    }
}

static void
handle_return_list(ArrayList *parameter_list, TypeDescriptor *caller_return_type_list,
                   TypeDescriptor *callee_return_type_list,
                   size_t num_returns,
                   char *module_name, int call_line, int defined_line) {
    // Remember that both ArrayVar and Var Nodes can be in VariableListNode
    // Prepare caller's descriptor from the parameter list; report undeclared error if necessary (i.e. not found in ST)
    for (size_t i = 0; i < num_returns; ++i) {
        TypeDescriptor caller_i = caller_return_type_list[i];
        TypeDescriptor callee_i = callee_return_type_list[i];
        if (caller_i.form == TYPE_UNDEFINED) {
            ASTNode **parameter = get_element_at_index_arraylist(parameter_list, i);
            char *identifier = GET_IDENTIFIER_FROM(*parameter);
            report_undeclared_identifier_in_call(identifier, call_line, (int) i + 1);
            continue;
        }

        if (!are_equivalent_type_descriptors(caller_i, callee_i)) {
            report_different_descriptor(call_line, module_name, defined_line, (int) i + 1,
                                        caller_i, callee_i, IN_RETURN);
        }
    }
}


static void report_wrong_number_in_call(int call_line, char *module_name, int defined_line, size_t expected,
                                        size_t actual, Where where, ManyOrFew many_or_few) {
    char error_message[256];
    if (where == IN_PARAMETER) {
        if (many_or_few == TOO_FEW)
            snprintf(error_message, sizeof error_message,
                     "Semantic Error on Line #%d: Too few arguments to function call, expected %zu, have %zu."
                     "'%s' defined on line #%d\n",
                     call_line, expected, actual, module_name, defined_line);
        else
            snprintf(error_message, sizeof error_message,
                     "Semantic Error on Line #%d: Too many arguments to function call, expected %zu, have %zu."
                     "'%s' defined on line #%d\n",
                     call_line, expected, actual, module_name, defined_line);
    } else {
        if (many_or_few == TOO_FEW)
            snprintf(error_message, sizeof error_message,
                     "Semantic Error on Line #%d: Too few arguments in function call assignment LHS, expected %zu, have %zu."
                     "'%s' defined on line #%d\n",
                     call_line, expected, actual, module_name, defined_line);
        else
            snprintf(error_message, sizeof error_message,
                     "Semantic Error on Line #%d: Too many arguments in function call assignment LHS, expected %zu, have %zu."
                     "'%s' defined on line #%d\n",
                     call_line, expected, actual, module_name, defined_line);
    }
    report_error(SEMANTIC_ERROR, error_message);
}

static void report_different_descriptor(int call_line, char *module_name, int defined_line, int param_number,
                                        TypeDescriptor caller_descriptor,
                                        TypeDescriptor callee_descriptor, Where where) {
    char error_message[256];
    char *caller_string = type_descriptor_to_string(caller_descriptor);
    char *callee_string = type_descriptor_to_string(callee_descriptor);
    if (where == IN_PARAMETER) {
        snprintf(error_message, sizeof error_message,
                 "Semantic Error on Line #%d Parameter #%d: Passing '%s' to parameter of incompatible type '%s'."
                 "'%s' defined on line #%d\n",
                 call_line, param_number, caller_string, callee_string, module_name, defined_line);
    } else {
        snprintf(error_message, sizeof error_message,
                 "Semantic Error on Line #%d Parameter #%d: Initializing '%s' with incompatible type '%s' returned by"
                 " module '%s' defined on line #%d\n",
                 call_line, param_number, caller_string, callee_string, module_name, defined_line);
    }

    report_error(SEMANTIC_ERROR, error_message);
    free(callee_string);
    free(caller_string);
}

static void report_undeclared_identifier_in_call(char *identifier, int call_line, int param_number) {
    char error_message[256];
    snprintf(error_message, sizeof error_message,
             "Semantic Error on Line #%d Parameter #%d: Use of undeclared identifier '%s'.\n",
             call_line, param_number, identifier);
    report_error(SEMANTIC_ERROR, error_message);
}

static void report_out_of_bounds_error(char *identifier, int line_number) {
    char error_message[256];
    snprintf(error_message, sizeof error_message, "Semantic Error on Line #%d:"
                                                  " Index out of bound for array identifier '%s'\n",
             line_number, identifier);
    report_error(SEMANTIC_ERROR, error_message);
}

