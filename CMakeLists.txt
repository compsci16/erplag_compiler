cmake_minimum_required(VERSION 3.21)
project(compiler C)
#add_compile_options(-fsanitize=address,undefined -g -Wall -Wextra -Wpedantic -Wconversion)
add_compile_options(-fsanitize=address,undefined -g)
#add_compile_options(-g)
add_link_options(-fsanitize=address,undefined)

set(CMAKE_C_STANDARD 17)

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address,undefined")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
include_directories(.)
include_directories(datastructures)
include_directories(datastructures/hashmap)
include_directories(datastructures/hashset)
include_directories(datastructures/arraylist)
include_directories(datastructures/linkedlist)
include_directories(datastructures/hash)
include_directories(datastructures/stack)
include_directories(compiler/lexer)
include_directories(compiler/lexer/core)
include_directories(compiler/lexer/dfa)
include_directories(compiler/parser/core)
include_directories(compiler/parser/core/ast)
include_directories(compiler/parser/core/parse_table)
include_directories(compiler/parser/core/parse_tree)
include_directories(compiler/parser/core/interface)
include_directories(compiler/lexer/constructor)
include_directories(compiler/parser)
include_directories(utility)

set(ARRAY_LIST datastructures/arraylist/arraylist.c)
set(HASH_MAP datastructures/hashmap/hashmap.c)
set(HASH_SET datastructures/hashset/hashset.c)
set(LINKED_LIST datastructures/linkedlist/linkedlist.c)
set(STACK datastructures/stack/stack.c)

set(UTILITY utility/utility.c utility/string_def.c utility/int.c utility/filehandler.h utility/filehandler.c)

set(ERROR utility/errors.h)
set(DS_FUNCTION_PTR utility/function_ptrs.h)
set(PARSER_GRAMMAR compiler/parser/grammar/first_follow/first.c compiler/parser/grammar/first_follow/follow.c compiler/parser/grammar/cfg.c)
set(PARSER_CORE compiler/parser/core/parse_table/parse_table.c compiler/parser/core/parse_tree/parse_tree.c compiler/parser/core/interface/parser.c)
set(LEXER_CORE compiler/lexer/core/lexer.c compiler/lexer/dfa/dfa.c)
set(LEXER_POPULATOR compiler/lexer/constructor/dfa_constructor.c compiler/lexer/constructor/lexer_constructor.c)
set(LEXER_DFA compiler/lexer/dfa/dfa.c compiler/lexer/dfa/checkers.c)
set(SYMBOL_TABLE compiler/parser/core/ast/nodes/program.h compiler/parser/core/ast/nodes/statements.h compiler/parser/core/ast/nodes/print_statement.c compiler/parser/core/ast/nodes/print_statement.h compiler/parser/core/ast/nodes/module_declarations.c compiler/parser/core/ast/nodes/module_declarations.h compiler/parser/core/ast/nodes/variable_node.c compiler/parser/core/ast/nodes/variable_node.h compiler/parser/core/ast/nodes/for.c compiler/parser/core/ast/nodes/for.h compiler/parser/core/ast/nodes/id.c compiler/parser/core/ast/nodes/id.h compiler/parser/core/ast/nodes/assignment_statement.c compiler/parser/core/ast/nodes/assignment_statement.h)
set(PARSER_GRAMMAR_CONSTRUCTOR compiler/parser/grammar/grammar_populator/grammar_constants.c compiler/parser/grammar/grammar_populator/constructor.c)

set(ALL_LEXER ${LEXER_CORE} ${LEXER_POPULATOR} ${LEXER_DFA} compiler/parser/core/ast/nodes/module_use_statement.c compiler/parser/core/ast/nodes/module_use_statement.c)
set(ALL_PARSER ${PARSER_CORE} ${PARSER_GRAMMAR} ${PARSER_GRAMMAR_CONSTRUCTOR} utility/comment_remover.h utility/comment_remover.c)
set(ALL_DATA_STRUCTURES ${ARRAY_LIST} ${HASH_MAP} ${HASH_SET} ${LINKED_LIST} ${STACK})
#add_executable(lexer_tester tests/lexer_test.c ${ALL_LEXER} ${ALL_DATA_STRUCTURES} ${UTILITY}
#        lexer/core/lexer.h lexer/dfa/dfa.h lexer/core/token.h)

#add_executable(parser_tester ${ALL_LEXER} ${ALL_PARSER} tests/parser_tester.c)
#add_executable(compiler ${ALL_LEXER} ${ALL_PARSER} ${UTILITY} ${ALL_DATA_STRUCTURES} compiler/main.c compiler/symbol_table/symbol_table.c compiler/symbol_table/symbol_table.h)
#add_executable(lexer_app ${ALL_LEXER} ${ALL_PARSER} ${UTILITY} ${ALL_DATA_STRUCTURES} compiler/lex_main.c compiler/symbol_table/symbol_table.c compiler/symbol_table/symbol_table.h)
add_executable(stage1exe ${ALL_PARSER} ${UTILITY}
        ${ALL_DATA_STRUCTURES} compiler/parser/grammar/grammar_populator/constructor.c compiler/parser/grammar/grammar_populator/constructor.h main.c)


add_executable(main2.c ${ALL_DATA_STRUCTURES} ${UTILITY} ${ALL_LEXER} ${ALL_PARSER} ${SYMBOL_TABLE}
        ./main2.c compiler/parser/core/ast/ast.c compiler/parser/core/ast/ast.h
        compiler/parser/core/ast/nodes/get_val.c compiler/parser/core/ast/nodes/statements.c
        compiler/parser/core/ast/nodes/program.c compiler/parser/grammar/first_follow/action_utility.c
        compiler/parser/grammar/first_follow/action_utility.h compiler/parser/core/ast/nodes/get_val.h
        compiler/parser/core/ast/nodes/integer_literal.c compiler/parser/core/ast/nodes/integer_literal.h
        compiler/parser/core/ast/nodes/real_literal.c compiler/parser/core/ast/nodes/real_literal.h
        compiler/parser/core/ast/nodes/boolean_literal.c compiler/parser/core/ast/nodes/boolean_literal.h
        compiler/parser/core/ast/nodes/module_node.c compiler/parser/core/ast/nodes/module_node.h
        compiler/parser/core/ast/nodes/modules_list_node.c compiler/parser/core/ast/nodes/modules_list_node.h
        compiler/parser/core/ast/nodes/variable_list.c compiler/parser/core/ast/nodes/variable_list.h
        compiler/parser/core/ast/nodes/array_variable.c compiler/parser/core/ast/nodes/array_variable.h
        compiler/parser/core/ast/nodes/switch.c compiler/parser/core/ast/nodes/switch.h compiler/parser/core/ast/nodes/case.c
        compiler/parser/core/ast/nodes/case.h compiler/lexer/core/token.c compiler/parser/core/ast/nodes/binary_operator.c
        compiler/parser/core/ast/nodes/binary_operator.h compiler/parser/core/ast/nodes/array.c
        compiler/parser/core/ast/nodes/array.h compiler/parser/core/ast/nodes/unary_operator.c
        compiler/parser/core/ast/nodes/unary_operator.h compiler/parser/core/ast/nodes/declare_variable_statement.c
        compiler/parser/core/ast/nodes/declare_variable_statement.h compiler/parser/core/ast/nodes/declare_array_statement.c
        compiler/parser/core/ast/nodes/declare_array_statement.h compiler/parser/core/ast/nodes/while.c
        compiler/parser/core/ast/nodes/while.h
        compiler/parser/core/ast/ast_def.h compiler/symbol_table/interface/symbol_table.c
        compiler/symbol_table/interface/symbol_table.h compiler/symbol_table/constructor/symbol_table_constructor.c
        compiler/parser/core/ast/nodes/variable_node.c compiler/lexer/constructor/dfa_constructor.h
        compiler/semantic_analyzer/semantic_analyzer.c compiler/error_reporter/error_report.c
        compiler/error_reporter/error_report.h compiler/parser/core/ast/node_utility.c
        compiler/parser/core/ast/node_utility.h
        compiler/code_generator/code_generator.c compiler/code_generator/code_generator.h compiler/symbol_table/constructor/symbol_table_constructor.h)