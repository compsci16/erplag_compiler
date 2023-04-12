#ifndef COMPILER_CHECKERS_H
#define COMPILER_CHECKERS_H

#include <stdbool.h>

typedef bool (*is_valid_char)(char);

const char *get_function_name(is_valid_char func);

is_valid_char string_to_function(const char *str);

bool is_small_alpha(char x);

bool is_capital_alpha(char x);

bool is_alpha(char x);

bool is_plus(char x);

bool is_minus(char x);

bool is_forward_slash(char x);

bool is_greater_than(char x);

bool is_less_than(char x);

bool is_star(char x);

bool is_colon(char x);

bool is_equal(char x);

bool is_exclamation(char x);

bool is_dot(char x);

bool is_new_line(char x);

bool is_tab(char x);

bool is_space(char x);

bool is_sq_bracket_open(char x);

bool is_sq_bracket_close(char x);

bool is_bracket_open(char x);

bool is_bracket_close(char x);

bool is_semicolon(char x);

bool is_comma(char x);

bool is_digit(char x);

bool is_underscore(char x);

bool is_eE(char x);

#endif //COMPILER_CHECKERS_H
