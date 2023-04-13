#include "symbol_table_constructor.h"
#include "utility.h"
#include "compiler/error_reporter/error_report.h"

#define CONSTRUCT_SYMBOL_TABLE_SUBTREE(node, parent) construct_symbol_table_helper((ASTNode *) (node), parent)
#define INTEGER_SIZE 2
#define REAL_SIZE 4
#define BOOLEAN_SIZE 1

static void construct_symbol_table_helper(ASTNode *node, SymbolTable *parent);

static void populate_module_symbol_table_entry(ModuleNode *module_node, SymbolTable *parent);


static void set_module_use_caller_parameter_type_descriptor_list(ModuleUseStatementNode *module_use_statement_node,
                                                                 SymbolTable *parent);

static void set_module_use_caller_return_type_descriptor_list(ModuleUseStatementNode *module_use_statement_node,
                                                              SymbolTable *parent);

static void report_undeclared_identifier_error(char *identifier, int line_number);


void construct_symbol_table(ProgramNode *root) {
    CONSTRUCT_SYMBOL_TABLE_SUBTREE(root, NULL);
}

static void construct_symbol_table_helper(ASTNode *node, SymbolTable *parent) {
    NODE_TYPE_TAG node_type = node->tag;
    switch (node_type) {
        case NODE_PROGRAM: {
            SymbolTable *symbol_table = create_symbol_table();
            ProgramNode *program_node = (ProgramNode *) node;
            program_node->global_symbol_table = symbol_table;
            // First Child Of Global Symbol Table is Module Declarations Table!
            program_node->module_declarations_symbol_table = create_symbol_table();
            add_child_symbol_table(symbol_table, program_node->module_declarations_symbol_table);
            ModuleDeclarationsNode *module_declarations_node = program_node->module_declarations_node;
            CONSTRUCT_SYMBOL_TABLE_SUBTREE(module_declarations_node, program_node->module_declarations_symbol_table);
            ModulesListNode *modules_list_node = program_node->modules_list_node;
            ArrayList *modules_list = modules_list_node->modules_list;
            for (size_t i = 0; i < modules_list->size; ++i) {
                ModuleNode **module_node = get_element_at_index_arraylist(modules_list, i);
                CONSTRUCT_SYMBOL_TABLE_SUBTREE(*module_node, symbol_table);
            }
            // need to call driver statements node here as well
            CONSTRUCT_SYMBOL_TABLE_SUBTREE(program_node->driver_module, program_node->global_symbol_table);
            break;
        }
        case NODE_ARRAY_VARIABLE: {
            ArrayVariableNode *array_variable_node = (ArrayVariableNode *) node;
            char *name = array_variable_node->id->id_token->lexeme;
            bool is_dynamic_array = array_variable_node->array_range.begin->token_type == ID ||
                                    array_variable_node->array_range.end->token_type == ID;
            SymbolTableEntry *other = search_in_symbol_table(parent, name);
            TypeDescriptor this_type = {
                    .form = TYPE_ARRAY,
                    .array_type={
                            .atomic_type = array_variable_node->type->token_type,
                            .array_range = array_variable_node->array_range,
                            .is_dynamic = is_dynamic_array
                    }
            };

            if (other != NULL) {
                TypeDescriptor other_type = other->type_descriptor;
                TypeForm other_type_atomic = other_type.array_type.atomic_type;
                TypeForm this_type_atomic = this_type.array_type.atomic_type;
                char err_msg[256];
                if (other_type_atomic != this_type_atomic ||
                    !are_equivalent_type_descriptors(other_type, this_type)) {
                    snprintf(err_msg, sizeof(err_msg),
                             "Semantic Error on Line #%d: "
                             "Redeclaration of %s with a different type. Previous declaration is on "
                             "Line Number %d.\n",
                             array_variable_node->id->id_token->line_number, name, other->line_number);
                    report_error(SEMANTIC_ERROR, err_msg);
                } else {
                    snprintf(err_msg, sizeof(err_msg),
                             "Semantic Error on Line #%d: Redeclaration of %s. Previous declaration is on "
                             "Line Number %d.\n",
                             array_variable_node->id->id_token->line_number, name, other->line_number);
                    report_error(SEMANTIC_ERROR, err_msg);
                }
                array_variable_node->symbol_table_entry = NULL;
                break;
            }

            INIT_SYMBOL_TABLE_ENTRY_PTR(symbol_table_entry);
            array_variable_node->symbol_table_entry = symbol_table_entry;
            symbol_table_entry->name = array_variable_node->id->id_token->lexeme;
            symbol_table_entry->line_number = array_variable_node->id->id_token->line_number;
            symbol_table_entry->type_descriptor = this_type;
            symbol_table_entry->is_array = true;
            int array_variable_width = 0;
            if (array_variable_node->array_range.begin->token_type == NUM &&
                array_variable_node->array_range.end->token_type == NUM) {
                symbol_table_entry->is_static = true;
                int left = atoi(array_variable_node->array_range.begin->lexeme);
                int right = atoi(array_variable_node->array_range.end->lexeme);
                if (array_variable_node->array_range.begin_sign->token_type == MINUS) {
                    left = ((-1) * left);
                }
                if (array_variable_node->array_range.end_sign->token_type == MINUS) {
                    right = ((-1) * right);
                }
                if (array_variable_node->type->token_type == INTEGER) {
                    parent->current_offset += 1 + (right - left + 1) * INTEGER_SIZE;
                    array_variable_width = 1 + (right - left + 1) * INTEGER_SIZE;
                    parent->total_data_size += 1 + (right - left + 1) * INTEGER_SIZE;
                } else if (array_variable_node->type->token_type == REAL) {
                    parent->current_offset += 1 + (right - left + 1) * REAL_SIZE;
                    array_variable_width = 1 + (right - left + 1) * REAL_SIZE;
                    parent->total_data_size += 1 + (right - left + 1) * REAL_SIZE;
                } else if (array_variable_node->type->token_type == BOOLEAN) {
                    parent->current_offset += 1 + (right - left + 1) * BOOLEAN_SIZE;
                    array_variable_width = 1 + (right - left + 1) * BOOLEAN_SIZE;
                    parent->total_data_size += 1 + (right - left + 1) * BOOLEAN_SIZE;
                }
            } else {
                parent->current_offset += 1;
                array_variable_width = 1;
            }
            symbol_table_entry->offset = parent->current_offset;
            symbol_table_entry->width = array_variable_width;
            symbol_table_entry->was_assigned = false;
            insert_in_symbol_table(parent, symbol_table_entry);
            break;
        }
        case NODE_MODULE: {
            ModuleNode *module_node = (ModuleNode *) node;
            populate_module_symbol_table_entry(module_node, parent);
            SymbolTableEntry *module_symbol_table_entry = module_node->symbol_table_entry;
            module_node->symbol_table_entry = module_symbol_table_entry;
            SymbolTable *function_entry_symbol_table = module_node->function_entry_symbol_table;
            function_entry_symbol_table->parent = parent;
            ArrayList *input_parameter_list = module_node->input_parameter_list_node->variable_nodes_list;
            ArrayList *output_parameter_list = module_node->output_parameter_list_node->variable_nodes_list;
            module_symbol_table_entry->type_descriptor.function_type.function_entry_symbol_table =
                    function_entry_symbol_table;

            for (size_t i = 0; i < module_symbol_table_entry->type_descriptor.function_type.num_parameters; ++i) {
                ASTNode **p_ast_node = get_element_at_index_arraylist(input_parameter_list, i);
                if ((*p_ast_node)->tag == NODE_ARRAY_VARIABLE) {
                    ArrayVariableNode *array_variable_node = (ArrayVariableNode *) *p_ast_node;
                    char *name = array_variable_node->id->id_token->lexeme;
                    bool is_dynamic_array = array_variable_node->array_range.begin->token_type == ID ||
                                            array_variable_node->array_range.end->token_type == ID;
                    SymbolTableEntry *other = search_in_symbol_table(function_entry_symbol_table, name);
                    TypeDescriptor this_type = {
                            .form = TYPE_ARRAY,
                            .array_type={
                                    .atomic_type = array_variable_node->type->token_type,
                                    .array_range = array_variable_node->array_range,
                                    .is_dynamic = is_dynamic_array
                            }
                    };

                    if (other != NULL) {
                        TypeDescriptor other_type = other->type_descriptor;
                        TypeForm other_type_atomic = other_type.array_type.atomic_type;
                        TypeForm this_type_atomic = this_type.array_type.atomic_type;
                        char err_msg[256];
                        if (other_type_atomic != this_type_atomic ||
                            !are_equivalent_type_descriptors(other_type, this_type)) {
                            snprintf(err_msg, sizeof(err_msg),
                                     "Semantic Error on Line #%d: "
                                     "Redeclaration of %s with a different type. Previous declaration is on "
                                     "Line Number %d.\n",
                                     array_variable_node->id->id_token->line_number, name, other->line_number);
                            report_error(SEMANTIC_ERROR, err_msg);
                        } else {
                            snprintf(err_msg, sizeof(err_msg),
                                     "Semantic Error on Line #%d: Redeclaration of %s. Previous declaration is on "
                                     "Line Number %d.\n",
                                     array_variable_node->id->id_token->line_number, name, other->line_number);
                            report_error(SEMANTIC_ERROR, err_msg);
                        }
                        array_variable_node->symbol_table_entry = NULL;
                        continue;
                    }

                    INIT_SYMBOL_TABLE_ENTRY_PTR(symbol_table_entry);
                    array_variable_node->symbol_table_entry = symbol_table_entry;
                    symbol_table_entry->name = array_variable_node->id->id_token->lexeme;
                    symbol_table_entry->line_number = array_variable_node->id->id_token->line_number;
                    symbol_table_entry->type_descriptor = this_type;
                    symbol_table_entry->is_array = true;
                    if (array_variable_node->array_range.begin->token_type == NUM &&
                        array_variable_node->array_range.end->token_type == NUM) {
                        symbol_table_entry->is_static = true;
                    }
                    symbol_table_entry->was_assigned = false;
                    function_entry_symbol_table->current_offset += 1 + 2 * INTEGER_SIZE;
                    symbol_table_entry->width = 1 + 2 * INTEGER_SIZE;
                    symbol_table_entry->offset = function_entry_symbol_table->current_offset;
                    insert_in_symbol_table(function_entry_symbol_table, symbol_table_entry);
                } else {
                    VariableNode *variable_node = (VariableNode *) *p_ast_node;
                    CONSTRUCT_SYMBOL_TABLE_SUBTREE(variable_node, function_entry_symbol_table);
                }
            }
            SymbolTable *function_scope_symbol_table = create_symbol_table();
            function_scope_symbol_table->current_offset = function_entry_symbol_table->current_offset;
            function_scope_symbol_table->parent = function_entry_symbol_table;
            function_scope_symbol_table->start_line = module_node->start_line;
            function_scope_symbol_table->end_line = module_node->end_line;
            for (size_t i = 0; i < module_symbol_table_entry->type_descriptor.function_type.num_returns; ++i) {
                VariableNode **variable_node = get_element_at_index_arraylist(output_parameter_list, i);
                CONSTRUCT_SYMBOL_TABLE_SUBTREE(*variable_node, function_scope_symbol_table);
            }

            CONSTRUCT_SYMBOL_TABLE_SUBTREE(module_node->statements_node, function_scope_symbol_table);

            module_node->function_scope_symbol_table = function_scope_symbol_table;
            char *module_name = module_node->module_id->id_token->lexeme;
            SymbolTableEntry *searched = search_in_symbol_table(parent, module_name);
            if (searched != NULL) {
                int err_line = module_node->module_id->id_token->line_number;
                char error_message[256];
                if (!are_equivalent_type_descriptors(searched->type_descriptor,
                                                     module_node->symbol_table_entry->type_descriptor)) {
                    snprintf(error_message, sizeof error_message,
                             "Semantic Error on Line #%d: Redeclaration of module '%s' with different type. %s%d\n",
                             err_line, module_name,
                             "Previous Declaration on line #", searched->line_number);
                } else {
                    snprintf(error_message, sizeof error_message,
                             "Semantic Error on Line #%d: Redeclaration of module '%s'. %s%d\n",
                             err_line, module_name,
                             "Previous Declaration on line #", searched->line_number);
                }
                report_error(SEMANTIC_ERROR, error_message);
                // module_node->symbol_table_entry = NULL;
                /* we let it build just for errors within this erroneous module that the programmer will likely
                 * rename */
                add_child_symbol_table(function_entry_symbol_table, function_scope_symbol_table);
                // destroy_symbol_table(function_entry_symbol_table);
                // destroy_symbol_table(function_scope_symbol_table);
            } else {
                add_child_symbol_table(parent, function_entry_symbol_table);
                insert_in_symbol_table(parent, module_symbol_table_entry);
                add_child_symbol_table(function_entry_symbol_table, function_scope_symbol_table);
            }
            break;
        }

        case NODE_STATEMENTS: {
            StatementsNode *statements_node = (StatementsNode *) node;
            ArrayList *statements_list = statements_node->statements_list;
            for (size_t i = 0; i < statements_list->size; ++i) {
                ASTNode **ast_node = get_element_at_index_arraylist(statements_list, i);
                switch ((*ast_node)->tag) {
                    case NODE_DECLARE_VARIABLE_STATEMENT:
                    case NODE_DECLARE_ARRAY_VARIABLE_STATEMENT:
                    case NODE_FOR:
                    case NODE_WHILE_STATEMENT:
                    case NODE_SWITCH:
                    case NODE_MODULE_USE_STATEMENT:
                    case NODE_GET_VALUE:
                    case NODE_ASSIGNMENT_STATEMENT:
                    case NODE_PRINT_STATEMENT: {
                        CONSTRUCT_SYMBOL_TABLE_SUBTREE(*ast_node, parent);
                        break;
                    }
                    default:
                        break;
                }
            }
            break;
        }
        case NODE_SWITCH: {
            SwitchStatementNode *switch_statement_node = (SwitchStatementNode *) node;
            CONSTRUCT_SYMBOL_TABLE_SUBTREE(switch_statement_node->id, parent);
            ArrayList *case_list = switch_statement_node->case_list;
            for (size_t i = 0; i < case_list->size; ++i) {
                CaseStatementNode **p_case_statement_node = get_element_at_index_arraylist(case_list, i);
                CaseStatementNode *case_statement_node = *p_case_statement_node;
                if (case_statement_node->tag == BOOLEAN_CONSTANT) {
                    case_statement_node->type_descriptor = (TypeDescriptor) {.form = TYPE_BOOLEAN};
                } else {
                    case_statement_node->type_descriptor = (TypeDescriptor) {.form = TYPE_INTEGER};
                }
                CONSTRUCT_SYMBOL_TABLE_SUBTREE(case_statement_node, parent);
            }
            if (switch_statement_node->default_case_statement_node != NULL) {
                CONSTRUCT_SYMBOL_TABLE_SUBTREE(switch_statement_node->default_case_statement_node,
                                               parent);
            }
            break;
        }
        case NODE_CASE: {
            CaseStatementNode *case_statement_node = (CaseStatementNode *) node;
            case_statement_node->case_symbol_table = create_symbol_table();
            SymbolTable *case_symbol_table = case_statement_node->case_symbol_table;
            case_symbol_table->current_offset = parent->current_offset;
            case_symbol_table->start_line = case_statement_node->start_line;
            case_symbol_table->end_line = case_statement_node->end_line;
            case_symbol_table->parent = parent;
            add_child_symbol_table(parent, case_symbol_table);
            CONSTRUCT_SYMBOL_TABLE_SUBTREE(case_statement_node->statements_node, case_symbol_table);
            break;
        }
        case NODE_VARIABLE: {
            VariableNode *variable_node = (VariableNode *) node;
            char *name = variable_node->id->id_token->lexeme;
            int line_num = variable_node->id->id_token->line_number;
            SymbolTableEntry *other = search_in_symbol_table(parent, name);
            if (other != NULL) {
                TypeDescriptor other_type = other->type_descriptor;
                // why cast: Because variable node is only for Atomic Type entries!
                TypeForm other_type_atomic = other_type.form;
                TypeForm this_type = variable_node->type->token_type;
                char err_msg[256];
                if (other_type_atomic != this_type) {
                    snprintf(err_msg, sizeof(err_msg),
                             "Semantic Error on Line #%d: Redeclaration of '%s' with a different type. "
                             "Previous declaration is on "
                             "Line #%d.\n",
                             line_num, name, other->line_number);
                    report_error(SEMANTIC_ERROR, err_msg);
                } else {
                    snprintf(err_msg, sizeof(err_msg),
                             "Semantic Error on Line #%d: Redeclaration of '%s'."
                             "Previous declaration is on "
                             "Line #%d.\n",
                             line_num, name, other->line_number);
                    report_error(SEMANTIC_ERROR, err_msg);
                }
                variable_node->symbol_table_entry = NULL;
                break;
            }

            INIT_SYMBOL_TABLE_ENTRY_PTR(symbol_table_entry);
            symbol_table_entry->type_descriptor.form = variable_node->type->token_type;
            symbol_table_entry->name = variable_node->id->id_token->lexeme;
            symbol_table_entry->line_number = variable_node->id->id_token->line_number;
            symbol_table_entry->was_assigned = false;
            variable_node->symbol_table_entry = symbol_table_entry;
            int variable_width = 0;
            if (symbol_table_entry->type_descriptor.form == TYPE_INTEGER) {
                parent->current_offset += INTEGER_SIZE;
                parent->total_data_size += INTEGER_SIZE;
                variable_width = INTEGER_SIZE;
            } else if (symbol_table_entry->type_descriptor.form == TYPE_REAL) {
                parent->current_offset += REAL_SIZE;
                parent->total_data_size += REAL_SIZE;
                variable_width = REAL_SIZE;
            } else if (symbol_table_entry->type_descriptor.form == TYPE_BOOLEAN) {
                parent->current_offset += BOOLEAN_SIZE;
                parent->total_data_size += BOOLEAN_SIZE;
                variable_width = BOOLEAN_SIZE;
            }

            symbol_table_entry->offset = parent->current_offset;
            symbol_table_entry->width = variable_width;

            insert_in_symbol_table(parent, symbol_table_entry);
            break;
        }
        case NODE_FOR: {
            ForNode *for_node = (ForNode *) node;
            // Important to set for's parent symbol table
            SymbolTable *for_symbol_table = create_symbol_table();
            for_symbol_table->current_offset = parent->current_offset;
            for_symbol_table->start_line = for_node->start_line;
            for_symbol_table->end_line = for_node->end_line;
            for_symbol_table->parent = parent;
            SymbolTableEntry *symbol_table_entry_for_counter = malloc_safe(sizeof(SymbolTableEntry));
            for_symbol_table->current_offset += INTEGER_SIZE;
            *symbol_table_entry_for_counter = (struct SymbolTableEntry) {
                    .type_descriptor.form = TYPE_INTEGER,
                    .name = for_node->variable_node->id->id_token->lexeme,
                    .line_number = for_node->variable_node->id->id_token->line_number,
                    .offset = for_symbol_table->current_offset,
                    .is_array = false,
                    .is_static = false,
                    .was_assigned=false,
                    .width = INTEGER_SIZE
            };
            for_node->variable_node->id->offset = for_symbol_table->current_offset;
            for_symbol_table->total_data_size += INTEGER_SIZE;
            insert_in_symbol_table(for_symbol_table, symbol_table_entry_for_counter);
            add_child_symbol_table(parent, for_symbol_table);
            CONSTRUCT_SYMBOL_TABLE_SUBTREE(for_node->statements_node, for_symbol_table);
            for_node->for_symbol_table = for_symbol_table;
            break;
        }
        case NODE_WHILE_STATEMENT: {
            WhileStatementNode *while_statement_node = (WhileStatementNode *) node;
            SymbolTable *while_symbol_table = create_symbol_table();
            while_symbol_table->current_offset = parent->current_offset;
            while_symbol_table->start_line = while_statement_node->start_line;
            while_symbol_table->end_line = while_statement_node->end_line;
            while_symbol_table->parent = parent;
            while_statement_node->while_symbol_table = while_symbol_table;
            add_child_symbol_table(parent, while_symbol_table);
            CONSTRUCT_SYMBOL_TABLE_SUBTREE(while_statement_node->statements_node, while_symbol_table);
            CONSTRUCT_SYMBOL_TABLE_SUBTREE(while_statement_node->predicate, while_symbol_table); //
            break;
        }

        case NODE_DECLARE_VARIABLE_STATEMENT: {
            DeclareVariableStatementNode *declare_variable_statement_node = (DeclareVariableStatementNode *) node;
            ArrayList *variable_nodes = declare_variable_statement_node->variable_nodes;
            size_t num_vars_declared = variable_nodes->size;
            SymbolTableEntry **records = malloc_safe(sizeof(SymbolTableEntry *) * (num_vars_declared + 1));
            records[num_vars_declared] = NULL;
            for (size_t i = 0; i < num_vars_declared; ++i) {
                VariableNode **variable_node = get_element_at_index_arraylist(variable_nodes, i);
                CONSTRUCT_SYMBOL_TABLE_SUBTREE(*variable_node, parent);
                if ((*variable_node)->symbol_table_entry != NULL) {
                    records[i] = (*variable_node)->symbol_table_entry;
                }
            }
            break;
        }
        case NODE_DECLARE_ARRAY_VARIABLE_STATEMENT: {
            DeclareArrayVariableStatementNode *declare_array_variable_statement_node =
                    (DeclareArrayVariableStatementNode *) node;
            ArrayList *array_variable_nodes = declare_array_variable_statement_node->array_variable_nodes;
            size_t num_array_vars_declared = array_variable_nodes->size;
            SymbolTableEntry **records = malloc_safe(sizeof(SymbolTableEntry *) * (num_array_vars_declared + 1));
            records[num_array_vars_declared] = NULL;
            for (size_t i = 0; i < num_array_vars_declared; ++i) {
                ArrayVariableNode **p_array_variable_node = get_element_at_index_arraylist(array_variable_nodes, i);
                CONSTRUCT_SYMBOL_TABLE_SUBTREE(*p_array_variable_node, parent);
                if ((*p_array_variable_node)->symbol_table_entry != NULL) {
                    records[i] = (*p_array_variable_node)->symbol_table_entry;
                }
            }
            break;
        }
        case NODE_VARIABLE_LIST:
        case NODE_ARRAY: {
            ArrayNode *array_node = (ArrayNode *) node;
            IDNode *id_node = array_node->array_id;
            SymbolTableEntry *symbol_table_entry = search_in_cactus_stack(parent, id_node->id_token->lexeme);
            if (symbol_table_entry == NULL) {
                id_node->identifier_type_descriptor = (TypeDescriptor) {.form = TYPE_UNDEFINED};
                Token *id_token = id_node->id_token;
                report_undeclared_identifier_error(id_token->lexeme, id_token->line_number);
            } else {
                id_node->identifier_type_descriptor = symbol_table_entry->type_descriptor;
                id_node->offset = symbol_table_entry->offset;
            }
            id_node->symbol_table = parent;
            CONSTRUCT_SYMBOL_TABLE_SUBTREE(array_node->array_expression, parent);
            break;
        }
        case NODE_BINARY_OPERATOR: {
            BinaryOperatorNode *binary_operator_node = (BinaryOperatorNode *) node;
            CONSTRUCT_SYMBOL_TABLE_SUBTREE(binary_operator_node->left, parent);
            CONSTRUCT_SYMBOL_TABLE_SUBTREE(binary_operator_node->right, parent);
            break;
        }
        case NODE_UNARY_OPERATOR: {
            UnaryOperatorNode *unary_operator_node = (UnaryOperatorNode *) node;
            CONSTRUCT_SYMBOL_TABLE_SUBTREE(unary_operator_node->operand, parent);
            break;
        }
        case NODE_ID: {
            IDNode *id_node = (IDNode *) node;
            SymbolTableEntry *symbol_table_entry = search_in_cactus_stack(parent, id_node->id_token->lexeme);
            if (symbol_table_entry == NULL) {
                id_node->identifier_type_descriptor = (TypeDescriptor) {.form = TYPE_UNDEFINED};
                Token *id_token = id_node->id_token;
                report_undeclared_identifier_error(id_token->lexeme, id_token->line_number);
            } else {
                id_node->identifier_type_descriptor = symbol_table_entry->type_descriptor;
                id_node->offset = symbol_table_entry->offset;
            }
            id_node->symbol_table = parent;
            break;
        }
        case NODE_PRINT_STATEMENT: {
            PrintStatementNode *print_statement_node = (PrintStatementNode *) node;
            if (print_statement_node->print_item->tag == NODE_ID) {
                CONSTRUCT_SYMBOL_TABLE_SUBTREE(print_statement_node->print_item, parent);
            }
            break;
        }
        case NODE_REAL_LITERAL:
            break;
        case NODE_MODULE_LIST:
            break;
        case NODE_BOOLEAN_LITERAL:
            break;
        case NODE_GET_VALUE: {
            GetValNode *get_val_node = (GetValNode *) node;
            CONSTRUCT_SYMBOL_TABLE_SUBTREE(get_val_node->id, parent);
            SymbolTableEntry *symbol_table_entry = search_in_cactus_stack(parent,
                                                                          get_val_node->id->id_token->lexeme);
            if (symbol_table_entry != NULL) {
                symbol_table_entry->was_assigned = true;
            }
            break;
        }
        case NODE_INTEGER_LITERAL: {

            break;
        }
        case NODE_MODULE_DECLARATIONS: {
            ModuleDeclarationsNode *module_declarations_node = (ModuleDeclarationsNode *) node;
            ArrayList *id_list = module_declarations_node->module_ids;
            for (size_t i = 0; i < id_list->size; ++i) {
                IDNode **p_id_node = get_element_at_index_arraylist(id_list, i);
                IDNode *id_node = *p_id_node;
                SymbolTableEntry *symbol_table_entry = malloc_safe(sizeof(SymbolTableEntry));
                symbol_table_entry->line_number = id_node->id_token->line_number;
                symbol_table_entry->type_descriptor = (TypeDescriptor) {.form = TYPE_FUNCTION};
                symbol_table_entry->name = id_node->id_token->lexeme;
                symbol_table_entry->was_assigned = false;
                symbol_table_entry->is_static = false;
                symbol_table_entry->is_array = false;
                insert_in_symbol_table(parent, symbol_table_entry);
            }
            break;
        }
        case NODE_ASSIGNMENT_STATEMENT: {
            AssignmentStatementNode *assignment_statement_node = (AssignmentStatementNode *) node;
            CONSTRUCT_SYMBOL_TABLE_SUBTREE(assignment_statement_node->lhs_node, parent);
            CONSTRUCT_SYMBOL_TABLE_SUBTREE(assignment_statement_node->rhs_node, parent);
            if (assignment_statement_node->lhs_node->tag == NODE_ID) {
                IDNode *id_node = (IDNode *) assignment_statement_node->lhs_node;
                SymbolTableEntry *symbol_table_entry = search_in_cactus_stack(parent, id_node->id_token->lexeme);
                if (symbol_table_entry != NULL) {
                    symbol_table_entry->was_assigned = true;
                }
            }
            break;
        }
        case NODE_MODULE_USE_STATEMENT: {
            ModuleUseStatementNode *module_use_statement_node = (ModuleUseStatementNode *) node;
            module_use_statement_node->parent_symbol_table = parent;
            module_use_statement_node->caller_type_descriptor = (TypeDescriptor) {.form = TYPE_FUNCTION};
            set_module_use_caller_parameter_type_descriptor_list(module_use_statement_node, parent);
            set_module_use_caller_return_type_descriptor_list(module_use_statement_node, parent);

            ArrayList *return_list = module_use_statement_node->result_id_list;
            for (size_t i = 0; i < return_list->size; ++i) {
                IDNode **r = get_element_at_index_arraylist(return_list, i);
                SymbolTableEntry *symbol_table_entry = search_in_cactus_stack(parent, (*r)->id_token->lexeme);
                if (symbol_table_entry != NULL) {
                    symbol_table_entry->was_assigned = true;
                }
            }

            break;
        }
        case FINAL_ENTRY_COUNT_OF_NODE_TYPES:
        default:
            break;
    }
}

static void set_module_use_caller_parameter_type_descriptor_list(ModuleUseStatementNode *module_use_statement_node,
                                                                 SymbolTable *parent) {
    ArrayList *parameter_list = module_use_statement_node->parameter_list;
    size_t num_parameters = parameter_list->size;
    TypeDescriptor *parameter_types = malloc_safe(num_parameters * sizeof(TypeDescriptor));

    for (size_t i = 0; i < num_parameters; ++i) {
        ASTNode **p_ast_node = get_element_at_index_arraylist(parameter_list, i);
        char *parameter_name;
        if ((*p_ast_node)->tag == NODE_ARRAY) {
            IDNode *id_node = ((ArrayNode *) *p_ast_node)->array_id;
            parameter_name = id_node->id_token->lexeme;
        } else {
            Token *id = ((IDNode *) *p_ast_node)->id_token;
            parameter_name = id->lexeme;
        }
        SymbolTableEntry *parameter_symbol_table_entry = search_in_cactus_stack(parent, parameter_name);
        if (parameter_symbol_table_entry != NULL) {
            parameter_types[i] = parameter_symbol_table_entry->type_descriptor;
        } else {
            // Case: Use of undeclared identifier!
            parameter_types[i] = (TypeDescriptor) {.form = TYPE_UNDEFINED};
        }
    }
    module_use_statement_node->caller_type_descriptor.function_type.parameter_types = parameter_types;
    module_use_statement_node->caller_type_descriptor.function_type.num_parameters = num_parameters;
}

static void set_module_use_caller_return_type_descriptor_list(ModuleUseStatementNode *module_use_statement_node,
                                                              SymbolTable *parent) {
    ArrayList *return_list = module_use_statement_node->result_id_list;
    size_t num_returns = return_list->size;
    TypeDescriptor *return_types = malloc_safe(num_returns * sizeof(TypeDescriptor));

    for (size_t i = 0; i < num_returns; ++i) {
        ASTNode **p_ast_node = get_element_at_index_arraylist(return_list, i);
        char *parameter_name;
        // Only id node possible on RHS by Syntax Rules (i.e. a[i] can't be on RHS)
        Token *id = ((IDNode *) *p_ast_node)->id_token;
        parameter_name = id->lexeme;
        SymbolTableEntry *parameter_symbol_table_entry = search_in_cactus_stack(parent, parameter_name);
        if (parameter_symbol_table_entry != NULL) {
            return_types[i] = parameter_symbol_table_entry->type_descriptor;
        } else {
            // Case: Use of undeclared identifier!
            return_types[i] = (TypeDescriptor) {.form = TYPE_UNDEFINED};
        }
    }
    module_use_statement_node->caller_type_descriptor.function_type.return_types = return_types;
    module_use_statement_node->caller_type_descriptor.function_type.num_returns = num_returns;
}

static void populate_module_symbol_table_entry(ModuleNode *module_node, SymbolTable *parent) {
    INIT_SYMBOL_TABLE_ENTRY_PTR(symbol_table_entry);
    symbol_table_entry->type_descriptor.form = TYPE_FUNCTION;
    symbol_table_entry->name = module_node->module_id->id_token->lexeme;
    symbol_table_entry->line_number = module_node->module_id->id_token->line_number;
    TypeDescriptor type_descriptor = {
            .form = TYPE_FUNCTION,
            .function_type = {
                    .num_returns =  module_node->output_parameter_list_node->variable_nodes_list->size,
                    .num_parameters = module_node->input_parameter_list_node->variable_nodes_list->size,
            }
    };
    type_descriptor.function_type.parameter_types =
            malloc_safe(sizeof(TypeDescriptor) * type_descriptor.function_type.num_parameters);
    type_descriptor.function_type.return_types = malloc_safe(sizeof(TypeDescriptor) *
                                                             type_descriptor.function_type.num_returns);

    type_descriptor.function_type.return_types =
            malloc_safe(sizeof(TypeDescriptor) * type_descriptor.function_type.num_returns);
    type_descriptor.function_type.parameter_types =
            malloc_safe(sizeof(TypeDescriptor) * type_descriptor.function_type.num_parameters);

    ArrayList *input_parameter_list = module_node->input_parameter_list_node->variable_nodes_list;
    ArrayList *output_parameter_list = module_node->output_parameter_list_node->variable_nodes_list;

    for (size_t i = 0; i < type_descriptor.function_type.num_returns; ++i) {
        VariableNode **variable_node = get_element_at_index_arraylist(output_parameter_list, i);
        type_descriptor.function_type.return_types[i].form = (*variable_node)->type->token_type;
    }

    for (size_t i = 0; i < type_descriptor.function_type.num_parameters; ++i) {
        ASTNode **p_ast_node = get_element_at_index_arraylist(input_parameter_list, i);

        if ((*p_ast_node)->tag == NODE_ARRAY_VARIABLE) {
            type_descriptor.function_type.parameter_types[i].form = TYPE_ARRAY;

            ArrayVariableNode *array_variable_node = (ArrayVariableNode *) *p_ast_node;

            type_descriptor.function_type.parameter_types[i] = (struct TypeDescriptor) {
                    .form = TYPE_ARRAY,
                    .array_type = {
                            .atomic_type = array_variable_node->type->token_type,
                            .array_range = array_variable_node->array_range,
                            .is_dynamic = array_variable_node->array_range.begin->token_type == ID ||
                                          array_variable_node->array_range.end->token_type == ID,
                    }
            };
        } else {
            VariableNode *variable_node = (VariableNode *) *p_ast_node;
            type_descriptor.function_type.parameter_types[i] = (struct TypeDescriptor) {
                    .form = (variable_node)->type->token_type
            };
        }

    }
    SymbolTable *module_symbol_table = create_symbol_table();
    module_symbol_table->start_line = module_node->start_line;
    module_symbol_table->end_line = module_node->end_line;
    module_node->symbol_table_entry = symbol_table_entry;
    type_descriptor.function_type.module_symbol_table = module_symbol_table;
    module_node->symbol_table_entry->type_descriptor = type_descriptor;
    module_node->function_entry_symbol_table = module_symbol_table;
    module_node->symbol_table_entry = symbol_table_entry;
}

static void report_undeclared_identifier_error(char *identifier, int line_number) {
    char error_message[256];
    snprintf(error_message, sizeof error_message, "Semantic Error on Line #%d: Use of undeclared identifier '%s'\n",
             line_number, identifier);
    report_error(SEMANTIC_ERROR, error_message);
}
