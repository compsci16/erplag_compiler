#include "code_generator.h"
#include "filehandler.h"
#include "utility.h"
#include "compiler/symbol_table/constructor/symbol_table_constructor.h"

#define GENERATE_CODE(X, Y) code_generator_helper((ASTNode* ) (X), Y)
FILE *global_out = NULL;
#define ASSEMBLY(X) fprintf(global_out,X)
#define ASSEMBLY2(X, Y) fprintf(global_out,X, Y)
#define ASSEMBLY3(X, Y, Z) fprintf(global_out, X, Y, Z)
#define MAX_OFFSET_STRING_LENGTH 4
#define LABEL_LENGTH_MAX 10

static void put_how_much_to_subtract_from_rbp_for_array_in_r8(SymbolTable *parent, ArrayNode *array_node);

static char *generate_temporary_label();

static SymbolTableEntry *generate_symbol_table_entry_for_new_temp(SymbolTable *symbol_table);

static char *generate_assembly_label();

static void check_bounds_from_r8(RangeInInt range_in_int);

static char *create_boolean_temporary_and_insert_it_in(SymbolTable *symbol_table);

static int get_offset_from_node(ASTNode *ast_node);

static void write_library_code();

static void generate_restore_stack_pointer_code();

static void generate_align_stack_code();

static void code_generator_helper(ASTNode *ast_node, SymbolTable *parent);

static int get_array_base_address(IDNode *array_id, SymbolTable *symbol_table);

static void generate_data_section();

static void generate_directives();

static void generate_text_section();

void generate_code_from(ProgramNode *root, char* output_path) {
    char *path = output_path;
    global_out = fopen(path, "w");
    if (!global_out) {
        report_file_error(path);
    }
    GENERATE_CODE(root, NULL);
}


static void code_generator_helper(ASTNode *ast_node, SymbolTable *parent) {
    switch (ast_node->tag) {
        case NODE_PROGRAM: {
            ProgramNode *program_node = (ProgramNode *) ast_node;
            generate_data_section();
            generate_directives();
            generate_text_section();
            write_library_code();
            GENERATE_CODE(program_node->driver_module, program_node->global_symbol_table);
            GENERATE_CODE(program_node->modules_list_node, program_node->global_symbol_table);
            break;
        }
        case NODE_ARRAY_VARIABLE:
            break;
        case NODE_BOOLEAN_LITERAL:
            break;
        case NODE_CASE: {
            CaseStatementNode *case_statement_node = (CaseStatementNode *) ast_node;
            ASSEMBLY("\t; allocate space for case\n");
            ASSEMBLY2("\tsub rsp, %d\n", case_statement_node->case_symbol_table->total_data_size);

            GENERATE_CODE(case_statement_node->statements_node, case_statement_node->case_symbol_table);

            ASSEMBLY("\t; deallocate space for case\n");
            ASSEMBLY2("\tadd rsp, %d\n", case_statement_node->case_symbol_table->total_data_size);
            break;
        }
        case NODE_GET_VALUE: {
            GetValNode *get_val_node = (GetValNode *) ast_node;
            IDNode *id_node = get_val_node->id;
            SymbolTable *id_symbol_table = id_node->symbol_table;
            SymbolTableEntry *symbol_table_entry = search_in_cactus_stack(id_symbol_table, id_node->id_token->lexeme);
            int offset = symbol_table_entry->offset;
            char *offset_str = malloc_safe(12 * sizeof(char));
            char *var_name = id_node->id_token->lexeme;
            snprintf(offset_str, 12, "%d", offset);
            TypeForm id_form = id_node->identifier_type_descriptor.form;
            if (id_form == TYPE_INTEGER) {
                ASSEMBLY(";getvalue integer\n");
                // Print get integer message
                ASSEMBLY("\tmov rbx, rsp\n");
                ASSEMBLY("\tand rsp, -16; align to 16 byte boundary\n");
                ASSEMBLY("\tlea rax, [rel int_message] ;load address of int_message to rax\n");
                ASSEMBLY("\tcall _print_message ; first argument is rax\n");
                ASSEMBLY("\tmov rsp, rbx; fix up the stack after each function call\n\n");

                // scan the integer of 2 bytes
                ASSEMBLY("\tmov rbx, rsp\n");
                ASSEMBLY("\tand rsp, -16\n");
                ASSEMBLY("\t;scanf(\"%%hd\", &i) so rdi gets first arg then rsi gets second arg\n");
                ASSEMBLY("\tlea rdi, [rel int_input_format]\n");
                ASSEMBLY3("\tlea rsi, [rbp - %s]; get address of integer to store for %s;\n", offset_str, var_name);
                ASSEMBLY("\tcall _scanf\n");
                ASSEMBLY("\tmov rsp, rbx; restore rsp\n\n");

                // print the obtained integer:
                ASSEMBLY("\tmov rbx, rsp ; save rsp\n");
                ASSEMBLY("\tand rsp, -16; align rsp to 16 bit boundary\n");
                ASSEMBLY("\tlea rdi, [rel int_output_format];\n");
                ASSEMBLY2("\tmovzx rsi, word [rbp - %s];\n", offset_str);
                ASSEMBLY("\tcall _printf\n");
                ASSEMBLY("\tmov rsp, rbx ; fix stack\n\n");

            } else if (id_form == TYPE_BOOLEAN) {
                ASSEMBLY(";getvalue boolean\n");
                generate_align_stack_code();
                ASSEMBLY("\tlea rax, [rel bool_message] ;load address of bool_message to rax\n");
                ASSEMBLY("\tcall _print_message ; first argument is rax\n");
                ASSEMBLY("\t;scanf(\"%%hd\", &i) so rdi gets first arg then rsi gets second arg\n");
                ASSEMBLY("\tlea rdi, [rel bool_input_format]\n");
                ASSEMBLY3("\tlea rsi, [rbp - %s]; get address of integer to store for %s;\n", offset_str, var_name);
                ASSEMBLY("\tcall _scanf\n");
                ASSEMBLY2("\tlea rdi, [rel bool_output_format];\n"
                          "\tmovzx rsi, word [rbp - %s];\n"
                          "\tcall _printf\n"
                          "\tmov rsp, rbx ; fix stack\n\n", offset_str);
                generate_restore_stack_pointer_code();
            } else if (id_form == TYPE_REAL) {
                ASSEMBLY(";getvalue real\n");
                generate_align_stack_code();
                generate_restore_stack_pointer_code();
            } else if (id_form == TYPE_ARRAY) {
                SymbolTableEntry *array_id_entry = search_in_cactus_stack(parent, var_name);
                TypeDescriptor array_type_descriptor = array_id_entry->type_descriptor;
                Range array_range = array_id_entry->type_descriptor.array_type.array_range;
                ASSEMBLY("\t;get value array\n");

                RangeInInt range_in_int = get_range_in_int_from(array_range);
                char message[96];
                int begin = range_in_int.begin;
                int end = range_in_int.end;
                int size = range_in_int.size;
                snprintf(message, 96, "Input: Enter %d array elements of integer type for range %d to %d",
                         size, begin, end);
                // array_enter_str db 'array enter', 0
                int message_length = strlen(message);
                for (int i = 0; i < message_length; ++i) {
                    char to_put = message[i];
                    ASSEMBLY3("\tmov byte[rel array_message + %d], '%c' ;\n", i, to_put);
                }
                ASSEMBLY3("\tmov byte[rel array_message + %d], %d ;\n", message_length, 0);
                ASSEMBLY("\tlea rax, [rel array_message]\n");
                ASSEMBLY("\tcall _print_message ; first argument is rax\n");

                int array_address_offset = array_id_entry->offset - array_id_entry->width + 1; //relative to rbp
                ASSEMBLY2("\t; get value array; base at rbp - %d\n", array_address_offset);
                if (array_type_descriptor.array_type.atomic_type == TYPE_INTEGER) {
                    for (int i = 1; i <= size; ++i) {
                        generate_align_stack_code();
                        ASSEMBLY("\tlea rax, [rel int_message] ;load address of int_message to rax\n");
                        ASSEMBLY("\tcall _print_message ; first argument is rax\n");
                        generate_restore_stack_pointer_code();

                        // scan the integer of 2 bytes
                        generate_align_stack_code();
                        int element_offset = array_address_offset + INTEGER_SIZE * i;
                        ASSEMBLY("\t;scanf(\"%%hd\", &i) so rdi gets first arg then rsi gets second arg\n");
                        ASSEMBLY("\tlea rdi, [rel int_input_format]\n");
                        ASSEMBLY3("\tlea rsi, [rbp - %d]; get address of integer to store for %s;\n", element_offset,
                                  var_name);
                        ASSEMBLY("\tcall _scanf\n");
                        generate_restore_stack_pointer_code();

                        // print the obtained integer:
                        generate_align_stack_code();
                        ASSEMBLY("\tlea rdi, [rel int_output_format];\n");
                        ASSEMBLY2("\tmovzx rsi, word [rbp - %d];\n", element_offset);
                        ASSEMBLY("\tcall _printf\n");
                        generate_restore_stack_pointer_code();
                    }
                }

            }

            break;
        }
        case NODE_INTEGER_LITERAL: {
            IntegerLiteralNode *integer_literal_node = (IntegerLiteralNode *) ast_node;
            SymbolTableEntry *symbol_table_entry = generate_symbol_table_entry_for_new_temp(parent);
            ASSEMBLY("\t;node integer literal, store it in temporary\n");
            ASSEMBLY2("\tsub rsp, %d\n", symbol_table_entry->width);
            ASSEMBLY2("\tmov r8, %s\n", integer_literal_node->num->lexeme);
            ASSEMBLY2("\tmov word[rbp - %d], r8w\n", symbol_table_entry->offset);
            integer_literal_node->result_offset = symbol_table_entry->offset;
            break;
        }
        case NODE_MODULE_DECLARATIONS:
            break;
        case NODE_MODULE_LIST: {
            ModulesListNode *modules_list_node = (ModulesListNode *) ast_node;
            ArrayList *list = modules_list_node->modules_list;
            for (size_t i = 0; i < list->size; ++i) {
                ModuleNode **p_module_node = get_element_at_index_arraylist(list, i);
                GENERATE_CODE(*p_module_node, parent);
            }
            break;
        }
        case NODE_MODULE: {
            ModuleNode *module_node = (ModuleNode *) ast_node;
            const int MAX_SIZE_STRING_LENGTH = 4;
            char *size_of_locals = malloc_safe(MAX_SIZE_STRING_LENGTH * sizeof(char));
            SymbolTable *function_entry_symbol_table = module_node->function_entry_symbol_table;
            SymbolTable *function_scope_symbol_table = module_node->function_scope_symbol_table;
            snprintf(size_of_locals, MAX_SIZE_STRING_LENGTH, "%d",
                     function_entry_symbol_table->total_data_size +
                     function_scope_symbol_table->total_data_size);
            char *module_name = module_node->module_id->id_token->lexeme;
            ASSEMBLY2("%s:\n", module_name);
            ASSEMBLY("\t;allocate space for module\n");
            ASSEMBLY("\tpush rbp\n");
            ASSEMBLY("\tmov rbp, rsp\n");
            ASSEMBLY2("\tsub rsp, %s\n", size_of_locals);
            GENERATE_CODE(module_node->statements_node, function_scope_symbol_table);
            char *size_of_locals_and_temporaries = malloc_safe(MAX_SIZE_STRING_LENGTH * sizeof(char));
            int sz_local_temp =
                    function_scope_symbol_table->total_data_size + function_entry_symbol_table->total_data_size;
            snprintf(size_of_locals_and_temporaries, MAX_SIZE_STRING_LENGTH, "%d", sz_local_temp);
            ASSEMBLY("\t;deallocate space from module\n");
            ASSEMBLY2("\tadd rsp, %s\n", size_of_locals_and_temporaries);
            ASSEMBLY("\tpop rbp\n");
            ASSEMBLY("\tret\n");
            break;
        }
        case NODE_PRINT_STATEMENT: {
            PrintStatementNode *print_statement_node = (PrintStatementNode *) ast_node;
            ASTNode *print_item = print_statement_node->print_item;
            switch (print_item->tag) {
                case NODE_INTEGER_LITERAL: {
                    IntegerLiteralNode *integer_literal_node = (IntegerLiteralNode *) print_statement_node->print_item;
                    char *integer_literal = integer_literal_node->num->lexeme;
                    ASSEMBLY(";print integer\n");
                    ASSEMBLY2("mov rbx, rsp ; preserve rsp\n"
                              "and rsp, -16\n"
                              "; printf(\"%%hd\", i); \n"
                              "lea rdi, [rel simple_int_output_format] ; Use new format string\n"
                              "mov eax, %s\n"
                              "movsx rsi, eax\n"
                              "call _printf \n"
                              "mov rsp, rbx ; restore rsp \n\n", integer_literal);
                    break;
                }
                case NODE_REAL_LITERAL: {
                    RealLiteralNode *real_literal_node = (RealLiteralNode *) print_statement_node->print_item;

                    break;
                }
                case NODE_BOOLEAN_LITERAL: {
                    BooleanLiteralNode *boolean_literal_node = (BooleanLiteralNode *) print_statement_node->print_item;
                    char *boolean_literal = boolean_literal_node->boolean_const->lexeme;
                    ASSEMBLY(";print boolean\n");
                    if (strcmp(boolean_literal, "true") == 0) {
                        ASSEMBLY(";print boolean\n"
                                 "mov rbx, rsp ; preserve rsp\n"
                                 "and rsp, -16\n"
                                 "; printf(\"%%s\", i); \n"
                                 "lea rdi, [rel simple_bool_output_format] ; Use new format string\n"
                                 "lea rsi, [rel bool_true]\n"
                                 "call _printf \n"
                                 "mov rsp, rbx ; restore rsp\n");
                    } else {
                        ASSEMBLY(";print boolean\n"
                                 "mov rbx, rsp ; preserve rsp\n"
                                 "and rsp, -16\n"
                                 "; printf(\"%%s\", i); \n"
                                 "lea rdi, [rel simple_bool_output_format] ; Use new format string\n"
                                 "lea rsi, [rel bool_false]\n"
                                 "call _printf \n"
                                 "mov rsp, rbx ; restore rsp\n");
                    }
                    break;
                }
                case NODE_ID: {
                    IDNode *id_node = (IDNode *) print_statement_node->print_item;
                    char *offset = malloc_safe(sizeof(char) * MAX_OFFSET_STRING_LENGTH);
                    TypeDescriptor type_descriptor = id_node->identifier_type_descriptor;
                    TypeForm form = type_descriptor.form;
                    snprintf(offset, MAX_OFFSET_STRING_LENGTH, "%d", id_node->offset);
                    switch (form) {
                        case TYPE_INTEGER: {
                            ASSEMBLY("\tmov rbx, rsp\n"
                                     "\tand rsp, -16\n");
                            ASSEMBLY2("\tmovsx rax, word[rbp - %s]\n", offset);
                            ASSEMBLY("\tcall _print_integer_id\n");
                            ASSEMBLY("\tmov rsp, rbx\n");
                            break;
                        }
                        case TYPE_BOOLEAN: {
                            generate_align_stack_code();
                            ASSEMBLY2("\tmovsx rax, byte[rbp - %s]\n", offset);
                            ASSEMBLY("\tcall _print_boolean_id\n");
                            generate_restore_stack_pointer_code();
                            break;
                        }
                        case TYPE_ARRAY: {
                            //   int array_address_offset = array_id_entry->offset - array_id_entry->width + 1;
                            char *var_name = id_node->id_token->lexeme;
                            SymbolTableEntry *symbol_table_entry = search_in_cactus_stack(parent, var_name);
                            int base_address_offset = symbol_table_entry->offset - symbol_table_entry->width + 1;
                            RangeInInt range_in_int = get_range_in_int_from(type_descriptor.array_type.array_range);
                            if (type_descriptor.array_type.atomic_type == TYPE_INTEGER) {
                                int size = range_in_int.size;
                                for (int i = 1; i <= size; ++i) {
                                    // scan the integer of 2 bytes
                                    int element_offset = base_address_offset + INTEGER_SIZE * i;
                                    generate_align_stack_code();
                                    ASSEMBLY("\tlea rdi, [rel int_output_format_same_line];\n");
                                    ASSEMBLY2("\tmovzx rsi, word [rbp - %d];\n", element_offset);
                                    ASSEMBLY("\tcall _printf\n");
                                    generate_restore_stack_pointer_code();
                                }
                            }
                            ASSEMBLY("\tlea rdi, [rel new_line_character_string];\n");
                            ASSEMBLY("\tcall _printf\n");
                            break;
                        }
                    }
                    free(offset);
                }
                default:
                    break;
            }
            break;
        }
        case NODE_REAL_LITERAL:
            break;
        case NODE_STATEMENTS: {
            StatementsNode *statements_node = (StatementsNode *) ast_node;
            for (int i = 0; i < statements_node->statements_list->size; ++i) {
                ASTNode **p_ast_node = get_element_at_index_arraylist(statements_node->statements_list, i);
                GENERATE_CODE(*p_ast_node, parent);
            }
            break;
        }
        case NODE_SWITCH: {
            ASSEMBLY("\t\t;switch statement\n");
            const int MAX_LABELS = 20;
            SwitchStatementNode *switch_statement_node = (SwitchStatementNode *) ast_node;
            size_t number_of_cases = switch_statement_node->case_list->size;
            char *case_labels[number_of_cases]; // 1 for default
            for (int i = 0; i < number_of_cases; ++i) {
                case_labels[i] = generate_assembly_label();
            }
            char *default_label = generate_assembly_label();
            char *test_label = generate_assembly_label();
            char *end_switch_label = generate_assembly_label();

            TypeDescriptor switch_td = switch_statement_node->id->identifier_type_descriptor;
            if (switch_td.form == TYPE_INTEGER) {
                int switch_id_offset = switch_statement_node->id->offset;
                ASSEMBLY2("\tmovsx r8, word[rbp - %d]; load switch test value (switch (a) , now a is in t)\n",
                          switch_id_offset);
                ASSEMBLY2("\tjmp %s ; goto test\n", test_label);
                for (int i = 0; i < number_of_cases; ++i) {
                    CaseStatementNode **current_case = get_element_at_index_arraylist(switch_statement_node->case_list,
                                                                                      i);
                    ASSEMBLY2("\t%s:\n", case_labels[i]);
                    GENERATE_CODE(*current_case, parent);
                    ASSEMBLY2("\tjmp %s\n", end_switch_label);
                }
                ASSEMBLY2("\t%s:\n", default_label);
                GENERATE_CODE(switch_statement_node->default_case_statement_node, parent);
                ASSEMBLY2("\tjmp %s\n", end_switch_label);

                ASSEMBLY2("\t%s: ; test_label\n", test_label);
                for (int i = 0; i < number_of_cases; ++i) {
                    CaseStatementNode **current_case =
                            get_element_at_index_arraylist(switch_statement_node->case_list,
                                                           i);
                    CaseStatementNode *case_node = *current_case;
                    char *integer_lexeme_to_compare = case_node->case_value->lexeme;
                    ASSEMBLY("\t; switch id value is stored in r8\n");
                    ASSEMBLY2("\t cmp r8, %s\n", integer_lexeme_to_compare);
                    ASSEMBLY2("\t jz %s \n", case_labels[i]);
                }
                ASSEMBLY("\t;default: \n");
                ASSEMBLY2("\tjmp %s\n", default_label);
                ASSEMBLY2("\t%s:\n", end_switch_label);
                return;
            } else {
                int switch_id_offset = switch_statement_node->id->offset;
                ASSEMBLY2("\tmovsx r8, byte[rbp - %d]; load switch test value (switch (a) , now a is in t)\n",
                          switch_id_offset);
                ASSEMBLY("\ttest r8, r8\n");
                char *jz_label = generate_assembly_label();
                ASSEMBLY2("\tjz %s\n", jz_label);
                ASSEMBLY("\tmov r8, 1\n");
                ASSEMBLY2("\t%s:\n", jz_label);

                ASSEMBLY2("\tjmp %s ; goto test\n", test_label);
                for (int i = 0; i < number_of_cases; ++i) {
                    CaseStatementNode **current_case = get_element_at_index_arraylist(switch_statement_node->case_list,
                                                                                      i);
                    ASSEMBLY2("\t%s:\n", case_labels[i]);
                    GENERATE_CODE(*current_case, parent);
                    ASSEMBLY2("\tjmp %s\n", end_switch_label);
                }

                ASSEMBLY2("\t%s: ; test_label\n", test_label);
                for (int i = 0; i < number_of_cases; ++i) {
                    CaseStatementNode **current_case =
                            get_element_at_index_arraylist(switch_statement_node->case_list,
                                                           i);
                    CaseStatementNode *case_node = *current_case;
                    int boolean_value = case_node->case_value->token_type == TRUE ? 1 : 0;

                    ASSEMBLY("\t; switch id value is stored in r8\n");
                    ASSEMBLY2("\t cmp r8, %d\n", boolean_value);
                    ASSEMBLY2("\t jz %s \n", case_labels[i]);
                }
                ASSEMBLY2("\t%s:\n", end_switch_label);
                return;
            }
            //ASSEMBLY("labe");
        }
            break;
        case NODE_VARIABLE_LIST:
            break;
        case NODE_VARIABLE:
            break;
        case NODE_FOR: {
            ForNode *for_node = (ForNode *) ast_node;
            SymbolTable *for_symbol_table = for_node->for_symbol_table;
            ASSEMBLY("\t; allocate space for 'for' construct\n");
            ASSEMBLY2("\t sub rsp, %d\n", for_symbol_table->total_data_size);
            int offset_of_counter = for_node->variable_node->id->offset;
            RangeInInt range_in_int = get_range_in_int_from(for_node->for_range);
            int counter_size = range_in_int.size;
            int begin_counter = range_in_int.begin;
            int end_counter = range_in_int.end;
            ASSEMBLY3("\tmov word[rbp - %d], %d; initialize counter with begin\n", offset_of_counter,
                      range_in_int.begin);
            char *loop_label = generate_assembly_label();
            char *out_of_loop_label = generate_assembly_label();

            int initial_symbol_table_size = for_symbol_table->total_data_size;

            ASSEMBLY2("\t%s:\n", loop_label);
            ASSEMBLY3("\tcmp word[rbp - %d], %d;\n", offset_of_counter, end_counter + 1);
//            ASSEMBLY("\ttest rcx, rcx\n");
            ASSEMBLY2("\tjz %s\n", out_of_loop_label);
//            ASSEMBLY("\tmov r15, rcx; save counter \n");
            GENERATE_CODE(for_node->statements_node, for_symbol_table);
//            ASSEMBLY("\tmov rcx, r15; restore counter\n");
            ASSEMBLY2("\tadd word[rbp - %d], 1\n", offset_of_counter);
            ASSEMBLY2("\tjmp %s\n", loop_label);
            ASSEMBLY2("\t%s:\n", out_of_loop_label);


            int final_symbol_table_size = for_symbol_table->total_data_size;

            int size_of_temps = final_symbol_table_size - initial_symbol_table_size;

            ASSEMBLY("\t; deallocate space for 'for' construct\n");
            ASSEMBLY2("\t add rsp, %d\n", initial_symbol_table_size + size_of_temps * range_in_int.size);


            break;
        }
        case NODE_ARRAY: {
            ArrayNode *array_node = (ArrayNode *) ast_node;
            put_how_much_to_subtract_from_rbp_for_array_in_r8(parent, array_node);
            //            SymbolTableEntry *symbol_table_entry =
//                    search_in_cactus_stack(parent, array_node->array_id->id_token->lexeme);
//            RangeInInt range_in_int = get_range_in_int_from(symbol_table_entry->type_descriptor.array_type.array_range);
//            GENERATE_CODE(array_node->array_expression, parent);
//            if (array_node->array_expression->tag == NODE_INTEGER_LITERAL) {
//                IntegerLiteralNode *literal_index_node = (IntegerLiteralNode *) array_node->array_expression;
//                ASSEMBLY2("\tmov r8, %s; store i of a[i] in r8\n",
//                          literal_index_node->num->lexeme);
//            } else {
//                int array_index_offset = get_offset_from_node(array_node->array_expression);
//                ASSEMBLY2("\tmovsx r8, word[rbp - %d]; store i of a[i] in r8\n",
//                          array_index_offset);
//            }
//            if (range_in_int.begin < 0) {
//                ASSEMBLY2("\tadd r8,%d\n", -range_in_int.begin);
//            } else {
//                ASSEMBLY2("\tsub r8,%d\n", range_in_int.begin);
//            }
//            ASSEMBLY("\tadd r8, 1\n");
//            ASSEMBLY("; element offset wrt rbp = base offset + IntegerSize * (index - starting_index + 1)\n");
//            ASSEMBLY2("\timul r8, %d\n", INTEGER_SIZE);
//            int array_base = get_array_base_address(array_node->array_id, parent);
//            ASSEMBLY2("\tadd r8, %d\n", array_base);
//            ASSEMBLY("\t; now r8 contains the index of the required element\n");

            char *temp = generate_temporary_label();
            SymbolTableEntry *symbol_table_entry_for_temp = malloc_safe(sizeof(SymbolTableEntry));
            parent->current_offset += INTEGER_SIZE;
            parent->total_data_size += INTEGER_SIZE;
            symbol_table_entry_for_temp->name = temp;
            symbol_table_entry_for_temp->type_descriptor = (TypeDescriptor) {.form = TYPE_INTEGER};
            symbol_table_entry_for_temp->offset = parent->current_offset;

            insert_in_symbol_table(parent, symbol_table_entry_for_temp);
            ASSEMBLY2("\tsub rsp, %d; allocate space a[i] in temporary\n", INTEGER_SIZE);
            ASSEMBLY("\tmov r12, rbp\n"
                     "\tsub r12, r8\n"
                     "\tmovsx r11, word[r12]\n");
            ASSEMBLY2("\tmov word[rbp - %d], r11w\n", symbol_table_entry_for_temp->offset);
            array_node->offset = symbol_table_entry_for_temp->offset;
            break;
        }
        case NODE_BINARY_OPERATOR: {
            BinaryOperatorNode *binary_operator_node = (BinaryOperatorNode *) ast_node;
            ASTNode *left = binary_operator_node->left;
            ASTNode *right = binary_operator_node->right;

            TypeDescriptor type_descriptor_left = get_type_descriptor(left);
            TypeDescriptor type_descriptor_right = get_type_descriptor(right);
            // Check if left and right operands are literals
            int is_left_literal = left->tag == NODE_INTEGER_LITERAL || left->tag == NODE_BOOLEAN_LITERAL;
            int is_right_literal = right->tag == NODE_INTEGER_LITERAL || right->tag == NODE_BOOLEAN_LITERAL;
            int is_left_array_node = left->tag == NODE_ARRAY;
            GENERATE_CODE(left, parent);
            GENERATE_CODE(right, parent);
            int offset_left = is_left_literal ? 0 : get_offset_from_node(left);
            int offset_right = is_right_literal ? 0 : get_offset_from_node(right);


            char offset_left_str[MAX_OFFSET_STRING_LENGTH] = {0};
            char offset_right_str[MAX_OFFSET_STRING_LENGTH] = {0};
            snprintf(offset_left_str, MAX_OFFSET_STRING_LENGTH, "%d", offset_left);
            snprintf(offset_right_str, MAX_OFFSET_STRING_LENGTH, "%d", offset_right);
            if (type_descriptor_left.form == TYPE_INTEGER) {
                if (is_left_literal) {
                    IntegerLiteralNode *left_int_literal = (IntegerLiteralNode *) left;
                    ASSEMBLY2("\tmov r8, %s\n",
                              left_int_literal->num->lexeme);  // Load the literal value directly into r8
                } else {
                    ASSEMBLY2("\tmovsx r8, word[rbp - %s] ; load left for plus\n", offset_left_str);
                }
            }

            if (type_descriptor_right.form == TYPE_INTEGER) {
                if (is_right_literal) {
                    IntegerLiteralNode *right_int_literal = (IntegerLiteralNode *) right;
                    ASSEMBLY2("\tmov r9, %s\n", right_int_literal->num->lexeme);
                } else {
                    ASSEMBLY2("\tmovsx r9, word[rbp - %s] ; load right for plus\n", offset_right_str);
                }
            }

            switch (binary_operator_node->operator->token_type) {
                case PLUS: {

                    ASSEMBLY("\tadd r9, r8\n");

                    if (binary_operator_node->binary_operator_type_descriptor.form == TYPE_INTEGER) {
                        // Create temporary
                        char *temp = generate_temporary_label();
                        INIT_SYMBOL_TABLE_ENTRY_PTR(temp_symbol_table_entry);
                        parent->current_offset += INTEGER_SIZE;
                        parent->total_data_size += INTEGER_SIZE;
                        temp_symbol_table_entry->name = temp;
                        temp_symbol_table_entry->type_descriptor = binary_operator_node->binary_operator_type_descriptor;
                        temp_symbol_table_entry->offset = parent->current_offset;
                        binary_operator_node->result_offset = parent->current_offset;
                        insert_in_symbol_table(parent, temp_symbol_table_entry);
                        ASSEMBLY2("\tsub rsp, %s\n", INTEGER_SIZE_STRING);
                        ASSEMBLY2("\tmov word[rbp - %d], r9w\n", binary_operator_node->result_offset);
                    }
                    break;
                }
                case MINUS: {
                    ASSEMBLY("\tsub r8, r9\n");
                    ASSEMBLY("\tmov r9, r8\n");
                    if (binary_operator_node->binary_operator_type_descriptor.form == TYPE_INTEGER) {
                        // Create temporary
                        char *temp = generate_temporary_label();
                        INIT_SYMBOL_TABLE_ENTRY_PTR(temp_symbol_table_entry);
                        parent->current_offset += INTEGER_SIZE;
                        parent->total_data_size += INTEGER_SIZE;
                        temp_symbol_table_entry->name = temp;
                        temp_symbol_table_entry->type_descriptor = binary_operator_node->binary_operator_type_descriptor;
                        temp_symbol_table_entry->offset = parent->current_offset;
                        binary_operator_node->result_offset = parent->current_offset;
                        insert_in_symbol_table(parent, temp_symbol_table_entry);
                        ASSEMBLY2("\tsub rsp, %s\n", INTEGER_SIZE_STRING);
                        ASSEMBLY2("\tmov word[rbp - %d], r9w\n", binary_operator_node->result_offset);
                    }
                    break;
                }
                case MUL: {
                    ASSEMBLY("\timul r9, r8\n");
                    if (binary_operator_node->binary_operator_type_descriptor.form == TYPE_INTEGER) {
                        // Create temporary
                        char *temp = generate_temporary_label();
                        INIT_SYMBOL_TABLE_ENTRY_PTR(temp_symbol_table_entry);
                        parent->current_offset += INTEGER_SIZE;
                        parent->total_data_size += INTEGER_SIZE;
                        temp_symbol_table_entry->name = temp;
                        temp_symbol_table_entry->type_descriptor = binary_operator_node->binary_operator_type_descriptor;
                        temp_symbol_table_entry->offset = parent->current_offset;
                        binary_operator_node->result_offset = parent->current_offset;
                        insert_in_symbol_table(parent, temp_symbol_table_entry);
                        ASSEMBLY2("\tsub rsp, %s; make space for multiplication temporary\n", INTEGER_SIZE_STRING);
                        ASSEMBLY2("\tmov word[rbp - %d], r9w\n", binary_operator_node->result_offset);
                    }
                    break;
                }
                case EQ: {
                    char *temp = create_boolean_temporary_and_insert_it_in(parent);
                    ASSEMBLY3("\tsub rsp, %s; allocate space for temporary boolean %s\n", BOOLEAN_SIZE_STRING, temp);

                    binary_operator_node->result_offset = parent->current_offset;
                    int result_offset = binary_operator_node->result_offset;

                    ASSEMBLY("\tcmp r8,r9\n");
                    char *label = generate_assembly_label();
                    ASSEMBLY2("\tje %s\n", label);
                    ASSEMBLY2("\tmov byte[rbp - %d], 0; move result into temporary ; NOT EQUAL\n", result_offset);
                    char *end_label = generate_assembly_label();
                    ASSEMBLY2("\tjmp %s\n", end_label);
                    ASSEMBLY2("\t%s: ; == true\n", label);
                    ASSEMBLY2("\tmov byte[rbp - %d], 1; move result into temporary\n", result_offset);
                    ASSEMBLY2("\t%s:\n", end_label);
                    free(end_label);
                    free(label);
                    // free(temp);
                    break;
                }
                case GT: {
                    char *temp = create_boolean_temporary_and_insert_it_in(parent);
                    ASSEMBLY3("\tsub rsp, %s; allocate space for temporary boolean %s\n", BOOLEAN_SIZE_STRING, temp);
                    binary_operator_node->result_offset = parent->current_offset;
                    int result_offset = binary_operator_node->result_offset;
                    ASSEMBLY("\tcmp r8,r9 ; r8 = a; r9 = b\n");
                    char *label = generate_assembly_label();
                    ASSEMBLY2("\tjg %s\n", label);
                    ASSEMBLY2("\tmov byte[rbp - %d], 0; move result into temporary ; a <= b\n", result_offset);
                    char *end_label = generate_assembly_label();
                    ASSEMBLY2("\tjmp %s\n", end_label);
                    ASSEMBLY2("\t%s: ; a > b\n", label);
                    ASSEMBLY2("\tmov byte[rbp - %d], 1; move result into temporary\n", result_offset);
                    ASSEMBLY2("\t%s:\n", end_label);
                    free(end_label);
                    free(label);
                    // free(temp);
                    break;
                }
                case GE: {
                    char *temp = create_boolean_temporary_and_insert_it_in(parent);
                    ASSEMBLY3("\tsub rsp, %s; allocate space for temporary boolean %s\n", BOOLEAN_SIZE_STRING, temp);

                    binary_operator_node->result_offset = parent->current_offset;
                    int result_offset = binary_operator_node->result_offset;
                    ASSEMBLY("\tcmp r8,r9 ; r8 = a; r9 = b\n");
                    char *label = generate_assembly_label();
                    ASSEMBLY2("\tjge %s\n", label);
                    ASSEMBLY2("\tmov byte[rbp - %d], 0; move result into temporary ; a < b\n", result_offset);
                    char *end_label = generate_assembly_label();
                    ASSEMBLY2("\tjmp %s\n", end_label);
                    ASSEMBLY2("\t%s: ; a >= b\n", label);
                    ASSEMBLY2("\tmov byte[rbp - %d], 1; move result into temporary\n", result_offset);
                    ASSEMBLY2("\t%s:\n", end_label);
                    free(end_label);
                    free(label);
                    // free(temp);
                    break;
                }
                case LT: {
                    char *temp = create_boolean_temporary_and_insert_it_in(parent);
                    ASSEMBLY3("\tsub rsp, %s; allocate space for temporary boolean %s\n", BOOLEAN_SIZE_STRING, temp);

                    binary_operator_node->result_offset = parent->current_offset;
                    int result_offset = binary_operator_node->result_offset;
                    ASSEMBLY("\tcmp r8,r9 ; r8 = a; r9 = b\n");
                    char *label = generate_assembly_label();
                    ASSEMBLY2("\tjl %s\n", label);
                    ASSEMBLY2("\tmov byte[rbp - %d], 0; move result into temporary ; a < b\n", result_offset);
                    char *end_label = generate_assembly_label();
                    ASSEMBLY2("\tjmp %s\n", end_label);
                    ASSEMBLY2("\t%s: ; a <= b\n", label);
                    ASSEMBLY2("\tmov byte[rbp - %d], 1; move result into temporary\n", result_offset);
                    ASSEMBLY2("\t%s:\n", end_label);
                    free(end_label);
                    free(label);
                    // free(temp);
                    break;
                }
                case LE: {
                    char *temp = create_boolean_temporary_and_insert_it_in(parent);
                    ASSEMBLY3("\tsub rsp, %s; allocate space for temporary boolean %s\n", BOOLEAN_SIZE_STRING, temp);
                    binary_operator_node->result_offset = parent->current_offset;
                    int result_offset = binary_operator_node->result_offset;
                    ASSEMBLY("\tcmp r8,r9 ; r8 = a; r9 = b\n");
                    char *label = generate_assembly_label();
                    ASSEMBLY2("\tjle %s\n", label);
                    ASSEMBLY2("\tmov byte[rbp - %d], 0; move result into temporary ; a <= b\n", result_offset);
                    char *end_label = generate_assembly_label();
                    ASSEMBLY2("\tjmp %s\n", end_label);
                    ASSEMBLY2("\t%s: ; a > b\n", label);
                    ASSEMBLY2("\tmov byte[rbp - %d], 1; move result into temporary\n", result_offset);
                    ASSEMBLY2("\t%s:\n", end_label);
                    free(end_label);
                    free(label);
                    // free(temp);
                    break;
                }
                case AND: {
                    char *temp = create_boolean_temporary_and_insert_it_in(parent);
                    ASSEMBLY3("\tsub rsp, %s; allocate space for temporary boolean %s\n", BOOLEAN_SIZE_STRING, temp);
                    binary_operator_node->result_offset = parent->current_offset;
                    int result_offset = binary_operator_node->result_offset;
                    if (type_descriptor_left.form == TYPE_BOOLEAN && type_descriptor_right.form == TYPE_BOOLEAN) {
                        if (!is_left_literal) {
                            ASSEMBLY2("\tmovsx r8, byte[rbp - %s] ; load left\n", offset_left_str);
                        } else {
                            BooleanLiteralNode *left_bool_literal = (BooleanLiteralNode *) left;
                            int value = get_boolean_value_from(left_bool_literal);
                            ASSEMBLY2("\tmov r8, %d ; load bool literal\n", value);
                        }

                        if (!is_right_literal) {
                            ASSEMBLY2("\tmovsx r9, byte[rbp - %s] ; load right\n", offset_right_str);
                        } else {
                            BooleanLiteralNode *right_bool_literal = (BooleanLiteralNode *) right;
                            int value = get_boolean_value_from(right_bool_literal);
                            ASSEMBLY2("\tmov r9, %d\n", value);
                        }
                    }

                    ASSEMBLY("\ttest r8, r8 ; is r8 = left 0 = false?\n");
                    char *jump_zero_label = generate_assembly_label();
                    ASSEMBLY2("\tjz %s ; jump if zero\n", jump_zero_label);
                    ASSEMBLY("\ttest r9, r9 ; is r9 = right = 0 = false? \n");
                    ASSEMBLY2("\tjz %s\n", jump_zero_label);
                    ASSEMBLY("\t; both are true\n");
                    ASSEMBLY2("\tmov byte[rbp - %d], 1\n", result_offset);
                    char *jump_end_label = generate_assembly_label();
                    ASSEMBLY2("\tjmp %s ; jump to end\n", jump_end_label);

                    ASSEMBLY2("\t%s: ; jump zero label\n", jump_zero_label);
                    ASSEMBLY2("\tmov byte[rbp - %d], 0\n", result_offset);

                    ASSEMBLY2("\t%s:\n", jump_end_label);

                    free(jump_zero_label);
                    free(jump_end_label);
                    break;
                }
                case OR: {
                    char *temp = create_boolean_temporary_and_insert_it_in(parent);
                    ASSEMBLY3("\tsub rsp, %s; allocate space for temporary boolean OR result %s\n", BOOLEAN_SIZE_STRING,
                              temp);
                    binary_operator_node->result_offset = parent->current_offset;
                    int result_offset = binary_operator_node->result_offset;
                    if (type_descriptor_left.form == TYPE_BOOLEAN && type_descriptor_right.form == TYPE_BOOLEAN) {
                        if (!is_left_literal) {
                            ASSEMBLY2("\tmovsx r8, byte[rbp - %s] ; load left\n", offset_left_str);
                        } else {
                            BooleanLiteralNode *left_bool_literal = (BooleanLiteralNode *) left;
                            int value = left_bool_literal->boolean_const->token_type == TRUE ? 1 : 0;
                            ASSEMBLY2("\tmov r8, %d ; load bool literal\n", value);
                        }

                        if (!is_right_literal) {
                            ASSEMBLY2("\tmovsx r9, byte[rbp - %s] ; load right\n", offset_right_str);
                        } else {
                            BooleanLiteralNode *right_bool_literal = (BooleanLiteralNode *) right;
                            int value = right_bool_literal->boolean_const->token_type == TRUE ? 1 : 0;
                            ASSEMBLY2("\tmov r9, %d\n", value);
                        }
                    }

                    char *jump_false_case_label = generate_assembly_label();
                    ASSEMBLY("\tor r8,r9\n");
                    ASSEMBLY2("\tjz %s ; jump false case\n", jump_false_case_label);
                    ASSEMBLY("; result of OR is true\n");
                    ASSEMBLY2("\tmov byte[rbp - %d], 1\n", result_offset);
                    char *jump_end_label = generate_assembly_label();
                    ASSEMBLY2("\tjmp %s\n", jump_end_label);

                    ASSEMBLY2("\t%s: ; result of OR is false do this\n", jump_false_case_label);
                    ASSEMBLY2("\tmov byte[rbp - %d], 0\n", result_offset);
                    ASSEMBLY2("\t%s:\n", jump_end_label);

                    free(jump_false_case_label);
                    free(jump_end_label);
                    break;
                }
            }
            break;
        }
        case NODE_UNARY_OPERATOR: {
            UnaryOperatorNode *unary_operator_node = (UnaryOperatorNode *) ast_node;
            int operator = unary_operator_node->operator->token_type;
            GENERATE_CODE(unary_operator_node->operand, parent);
            int operand_result_offset = get_offset_from_node(unary_operator_node->operand);

            if (operator == PLUS) {
                unary_operator_node->result_offset = operand_result_offset;
            } else {
                if (unary_operator_node->unary_operator_type_descriptor.form == TYPE_INTEGER) {
                    ASSEMBLY2("\tmovsx r8, word[rbp - %d]\n", operand_result_offset);
                    ASSEMBLY("\tneg r8 ; invert sign for unary minus\n");
                    SymbolTableEntry *symbol_table_entry = generate_symbol_table_entry_for_new_temp(parent);
                    ASSEMBLY2("\tsub rsp, %d ; allocate space for temp\n", symbol_table_entry->width);
                    ASSEMBLY2("\t mov word[rbp - %d], r8w\n", symbol_table_entry->offset);
                    unary_operator_node->result_offset = symbol_table_entry->offset;
                }
            }
            break;
        }
        case NODE_ID:
            break;
        case NODE_DECLARE_VARIABLE_STATEMENT:
            break;
        case NODE_DECLARE_ARRAY_VARIABLE_STATEMENT: {
            DeclareArrayVariableStatementNode *declare_array_variable_statement_node =
                    (DeclareArrayVariableStatementNode *) ast_node;
            for (int i = 0; i < declare_array_variable_statement_node->array_variable_nodes->size; ++i) {
                ArrayVariableNode **array_variable_node =
                        declare_array_variable_statement_node->array_variable_nodes->elements[i];
                GENERATE_CODE(*array_variable_node, parent);
            }
            break;
        }
        case NODE_ASSIGNMENT_STATEMENT: {
            AssignmentStatementNode *assignment_statement_node = (AssignmentStatementNode *) ast_node;


            GENERATE_CODE(assignment_statement_node->rhs_node, parent);
            int id_offset = get_offset_from_node(assignment_statement_node->lhs_node);
            bool is_lhs_array_node = assignment_statement_node->lhs_node->tag == NODE_ARRAY;
            if (is_lhs_array_node) {
                ArrayNode *array_node = (ArrayNode *) assignment_statement_node->lhs_node;
                GENERATE_CODE(array_node->array_expression, parent);
                put_how_much_to_subtract_from_rbp_for_array_in_r8(parent, array_node);
            }

            if (assignment_statement_node->rhs_node->tag == NODE_INTEGER_LITERAL) {
                IntegerLiteralNode *rhs_lit = (IntegerLiteralNode *) assignment_statement_node->rhs_node;
                if (!is_lhs_array_node) {
                    ASSEMBLY3("\tmov word[rbp - %d], %s\n", id_offset, rhs_lit->num->lexeme);
                } else {
                    ASSEMBLY("\tmov r11, rbp\n");
                    ASSEMBLY("\tsub r11, r8\n");
                    ASSEMBLY2("\tmov word[r11], %s\n", rhs_lit->num->lexeme);
                }
                return;
            } else if (assignment_statement_node->rhs_node->tag == NODE_BOOLEAN_LITERAL) {
                BooleanLiteralNode *rhs_lit = (BooleanLiteralNode *) assignment_statement_node->rhs_node;
                int val = get_boolean_value_from(rhs_lit);
                ASSEMBLY3("\tmov byte[rbp - %d], %d\n", id_offset, val);
                return;
            }

            int temporary_offset = get_offset_from_node(assignment_statement_node->rhs_node);

            char temporary_offset_string[MAX_OFFSET_STRING_LENGTH];
            char id_offset_string[MAX_OFFSET_STRING_LENGTH];
            snprintf(temporary_offset_string, MAX_OFFSET_STRING_LENGTH, "%d", temporary_offset);
            snprintf(id_offset_string, MAX_OFFSET_STRING_LENGTH, "%d", id_offset);
            TypeDescriptor rhs_type_descriptor = get_type_descriptor(assignment_statement_node->rhs_node);
            if (assignment_statement_node->lhs_node->tag == NODE_ID) {
                if (rhs_type_descriptor.form == TYPE_INTEGER) {
                    ASSEMBLY2("\tmovsx r8, word[rbp - %s]\n", temporary_offset_string);
                    ASSEMBLY2("\tmov word[rbp - %s], r8w\n", id_offset_string);
                } else if (rhs_type_descriptor.form == TYPE_BOOLEAN) {
                    ASSEMBLY2("\tmovsx r8, byte[rbp - %s]; boolean assignment\n", temporary_offset_string);
                    ASSEMBLY2("\tmov byte[rbp - %s], r8b ; boolean assignment\n", id_offset_string);
                }
            } else {
                if (rhs_type_descriptor.form == TYPE_INTEGER) {
                    // NODE_ARRAY
                    ASSEMBLY2("\tmovsx r12, word[rbp - %s]\n", temporary_offset_string);
                    ASSEMBLY("\tmov r11, rbp\n");
                    ASSEMBLY("\tsub r11, r8\n");
                    ASSEMBLY("\tmov word[r11], r12w\n");
                }
            }
            break;
        }
        case NODE_WHILE_STATEMENT:
            break;
        case NODE_MODULE_USE_STATEMENT:
            break;
        case FINAL_ENTRY_COUNT_OF_NODE_TYPES:
            break;
    }
}

static char *create_boolean_temporary_and_insert_it_in(SymbolTable *symbol_table) {
    // Create temporary
    char *temp = generate_temporary_label();
    INIT_SYMBOL_TABLE_ENTRY_PTR(temp_symbol_table_entry);
    symbol_table->current_offset += BOOLEAN_SIZE;
    symbol_table->total_data_size += BOOLEAN_SIZE;
    temp_symbol_table_entry->name = temp;
    temp_symbol_table_entry->type_descriptor = (TypeDescriptor) {.form = TYPE_BOOLEAN};
    temp_symbol_table_entry->offset = symbol_table->current_offset;
    insert_in_symbol_table(symbol_table, temp_symbol_table_entry);

    return temp;
}

static char *generate_temporary_label() {
    const int TEMP_LENGTH_MAX = 4;
    static int counter = 0;
    char *temp = malloc_safe(sizeof(TEMP_LENGTH_MAX));
    snprintf(temp, TEMP_LENGTH_MAX, "t%d", counter);
    ++counter;
    return temp;
}

static char *generate_assembly_label() {
    static int counter = 0;
    char *temp = malloc_safe(sizeof(char) * LABEL_LENGTH_MAX);
    snprintf(temp, LABEL_LENGTH_MAX, "label%d", counter);
    ++counter;
    return temp;
}

static SymbolTableEntry *generate_symbol_table_entry_for_new_temp(SymbolTable *symbol_table) {
    char *temp = generate_temporary_label();
    SymbolTableEntry *symbol_table_entry_for_temp = malloc_safe(sizeof(SymbolTableEntry));
    symbol_table->current_offset += INTEGER_SIZE;
    symbol_table->total_data_size += INTEGER_SIZE;
    symbol_table_entry_for_temp->width = INTEGER_SIZE;
    symbol_table_entry_for_temp->name = temp;
    symbol_table_entry_for_temp->type_descriptor = (TypeDescriptor) {.form = TYPE_INTEGER};
    symbol_table_entry_for_temp->offset = symbol_table->current_offset;
    insert_in_symbol_table(symbol_table, symbol_table_entry_for_temp);
    return symbol_table_entry_for_temp;
}

static void generate_data_section() {
    ASSEMBLY("SECTION .data\n");
    ASSEMBLY("int_message db 'Input: Enter an integer value:',0\n");
    ASSEMBLY("int_input_format db '%%hd', 0 ; for short as integer is of 2 bytes in erplag\n");
    ASSEMBLY("int_output_format_same_line db '%%hd ', 0\n");
    ASSEMBLY("new_line_character_string db  10, 0\n");
    ASSEMBLY("int_output_format db 'You entered: %%hd', 10, 0\n");
    ASSEMBLY("array_index_out_of_bounds_message db 'RUN TIME ERROR: Array Index Out of Bound', 10, 0\n");
    ASSEMBLY("simple_int_output_format db '%%hd', 10, 0\n");
    ASSEMBLY("simple_bool_output_format db '%%s', 0\n");
    ASSEMBLY("simple_string_output_format db '%%s', 0\n");
    ASSEMBLY("simple_real_output_format db '%%f', 10, 0\n");
    ASSEMBLY(
            "bool_message db 'Input: Enter a boolean value, where 0 is treated as false and anything else as true:',0\n"
            "bool_input_format db '%%hhd', 0 ; for boolean\n"
            "bool_output_format db 'You entered: %%hhd', 10, 0\n"
    );
    ASSEMBLY("bool_true db 'true',10, 0\n"
             "bool_false db 'false',10, 0\n");
    ASSEMBLY("array_message resb 64 ; not a constant\n");
}

static void put_how_much_to_subtract_from_rbp_for_array_in_r8(SymbolTable *parent, ArrayNode *array_node) {
    SymbolTableEntry *symbol_table_entry =
            search_in_cactus_stack(parent, array_node->array_id->id_token->lexeme);
    RangeInInt range_in_int = get_range_in_int_from(symbol_table_entry->type_descriptor.array_type.array_range);
    GENERATE_CODE(array_node->array_expression, parent);
    if (array_node->array_expression->tag == NODE_INTEGER_LITERAL) {
        IntegerLiteralNode *literal_index_node = (IntegerLiteralNode *) array_node->array_expression;
        ASSEMBLY2("\tmov r8, %s; store i of a[i] in r8\n",
                  literal_index_node->num->lexeme);
    } else {
        int array_index_offset = get_offset_from_node(array_node->array_expression);
        ASSEMBLY2("\tmovsx r8, word[rbp - %d]; store i of a[i] in r8\n",
                  array_index_offset);
    }


    check_bounds_from_r8(range_in_int);


    if (range_in_int.begin < 0) {
        ASSEMBLY2("\tadd r8,%d\n", -range_in_int.begin);
    } else {
        ASSEMBLY2("\tsub r8,%d\n", range_in_int.begin);
    }
    ASSEMBLY("\tadd r8, 1\n");
    ASSEMBLY("; element offset wrt rbp = base offset + IntegerSize * (index - starting_index + 1)\n");
    ASSEMBLY2("\timul r8, %d\n", INTEGER_SIZE);
    int array_base = get_array_base_address(array_node->array_id, parent);
    ASSEMBLY2("\tadd r8, %d\n", array_base);
    ASSEMBLY("\t; now r8 contains the index of the required element\n");
}

static void check_bounds_from_r8(RangeInInt range_in_int) {
    int begin = range_in_int.begin;
    int end = range_in_int.end;
    ASSEMBLY2("\tcmp r8, %d\n", begin);
    ASSEMBLY("\tjl exit_array_error\n");


    ASSEMBLY2("\tcmp r8, %d\n", end);
    ASSEMBLY("\tjg exit_array_error\n");
}

static int get_offset_from_node(ASTNode *ast_node) {
    switch (ast_node->tag) {
        case NODE_ARRAY_VARIABLE: {
            ArrayVariableNode *array_variable_node = (ArrayVariableNode *) ast_node;
            SymbolTableEntry *symbol_table_entry = array_variable_node->symbol_table_entry;
            if (symbol_table_entry->type_descriptor.array_type.is_dynamic) {

            }
            break;
        }
        case NODE_BOOLEAN_LITERAL:
            break;
        case NODE_CASE:
            break;
        case NODE_GET_VALUE:
            break;
        case NODE_INTEGER_LITERAL: {
            return ((IntegerLiteralNode *) ast_node)->result_offset;
        }
        case NODE_MODULE_DECLARATIONS:
            break;
        case NODE_MODULE:
            break;
        case NODE_MODULE_LIST:
            break;
        case NODE_PRINT_STATEMENT:
            break;
        case NODE_PROGRAM:
            break;
        case NODE_REAL_LITERAL:
            break;
        case NODE_STATEMENTS:
            break;
        case NODE_SWITCH: {


            break;
        }
        case NODE_VARIABLE_LIST:
            break;
        case NODE_VARIABLE:
            break;
        case NODE_FOR:
            break;
        case NODE_ARRAY: {
            ArrayNode *array_node = (ArrayNode *) ast_node;
            return array_node->offset;

        }
        case NODE_BINARY_OPERATOR: {
            return ((BinaryOperatorNode *) ast_node)->result_offset;
        }
        case NODE_UNARY_OPERATOR: {
            UnaryOperatorNode *unary_operator_node = (UnaryOperatorNode *) ast_node;
            return unary_operator_node->result_offset;
        }
        case NODE_ID: {
            return ((IDNode *) ast_node)->offset;
        }
        case NODE_DECLARE_VARIABLE_STATEMENT:
            break;
        case NODE_DECLARE_ARRAY_VARIABLE_STATEMENT:
            break;
        case NODE_ASSIGNMENT_STATEMENT:
            break;
        case NODE_WHILE_STATEMENT:
            break;
        case NODE_MODULE_USE_STATEMENT:
            break;
        case FINAL_ENTRY_COUNT_OF_NODE_TYPES:
            break;
    }
    return 100000;
}

static int get_array_base_address(IDNode *array_id, SymbolTable *symbol_table) {
    char *name = array_id->id_token->lexeme;
    SymbolTableEntry *array_id_entry = search_in_cactus_stack(symbol_table, name);
    int array_address_offset = array_id_entry->offset - array_id_entry->width + 1; //relative to rbp
    return array_address_offset;
}

static void generate_align_stack_code() {
    ASSEMBLY("\t;align stack\n");
    ASSEMBLY("\tmov rbx, rsp\n"
             "\tand rsp, -16\n");
}

static void generate_restore_stack_pointer_code() {
    ASSEMBLY("\t;restore stack\n");
    ASSEMBLY("\tmov rsp, rbx\n");
}

static void generate_directives() {
    ASSEMBLY("global  _main\n");
    ASSEMBLY("extern _puts\n");
    ASSEMBLY("extern _scanf\n");
    ASSEMBLY("extern _printf\n");
}

static void generate_text_section() {
    ASSEMBLY("SECTION .text\n");
    ASSEMBLY("_main:\n");
    ASSEMBLY("\tcall DRIVER\n");
    ASSEMBLY("\texit:\n");
    ASSEMBLY("\tmov rax, 0x02000001         ; system call for exit; CHANGE IN LINUX\n");
    ASSEMBLY("\txor rdi, rdi; exit code 0\n");
    ASSEMBLY("\tsyscall ;invoke OS to exit\n\n");

    ASSEMBLY("\texit_array_error:\n");
    generate_align_stack_code();
    ASSEMBLY("\tlea rdi, [rel simple_string_output_format]\n");
    ASSEMBLY("\tlea rsi, [rel array_index_out_of_bounds_message]\n");
    ASSEMBLY("\tcall _printf\n");
    generate_restore_stack_pointer_code();
    ASSEMBLY("\tmov rax, 0x02000001         ; system call for exit; CHANGE IN LINUX\n");
    ASSEMBLY("\txor rdi, rdi; exit code 0\n");
    ASSEMBLY("\tsyscall ;invoke OS to exit\n\n");
}


static void write_library_code() {
    ASSEMBLY(";first argument in rax\n"
             "_print_message:\n"
             "\tpush rdi \n"
             "\tlea  rdi, [rax]    ; First argument is address of message\n"
             "\tcall      _puts                   ; puts(message)\n"
             "\tpop rdi\n"
             "\tret\n");

    ASSEMBLY(";1st argument: integer to print in rax\n"
             ";printf(\"%%d\", i); \n"
             "_print_integer_id:\n"
             "\tmov r12, rsp; \n"
             "\n"
             "\tand rsp, -16\n"
             "\tlea rdi, [rel simple_int_output_format]; printf first arg %%hd\n"
             "\tmov rsi, rax ; printf second arg \n"
             "\tcall _printf\n"
             "\n"
             "\t;epilog\n"
             "\tmov rsp, r12\n"
             "\tret\n");

    ASSEMBLY("; assumes the boolean value is in rax ; 0 = false, anything else true\n"
             "_print_boolean_id:\n"
             "\ttest rax, rax\n"
             "\tjz printFalse\n"
             "\tlea rdi, [rel simple_bool_output_format]\n"
             "\tlea rsi, [rel bool_true]\n"
             "\tcall _printf\n"
             "\tret\n"
             "\tprintFalse: \n"
             "\t\tlea rdi, [rel simple_bool_output_format]\n"
             "\t\tlea rsi, [rel bool_false]\n"
             "\t\tcall _printf\n"
             "\t\tret\n");
}