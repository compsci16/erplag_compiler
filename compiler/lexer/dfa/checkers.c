#include<stdbool.h>
#include"checkers.h"
#include <stddef.h>
#include <string.h>

bool is_small_alpha(char x) {
    return (x >= 'a' && x <= 'z');
}

bool is_capital_alpha(char x) {
    return (x >= 'A' && x <= 'Z');
}

bool is_alpha(char x) {
    return is_small_alpha(x) || is_capital_alpha(x);
}

bool is_plus(char x) {
    return x == '+';
}

bool is_minus(char x) {
    return x == '-';
}

bool is_forward_slash(char x) {
    return x == '/';
}

bool is_greater_than(char x) {
    return x == '>';
}

bool is_less_than(char x) {
    return x == '<';
}

bool is_star(char x) {
    return x == '*';
}

bool is_colon(char x) {
    return x == ':';
}

bool is_equal(char x) {
    return x == '=';
}

bool is_exclamation(char x) {
    return x == '!';
}

bool is_dot(char x) {
    return x == '.';
}

bool is_new_line(char x) {
    return x == '\n';
}

bool is_tab(char x) {
    return x == '\t';
}

bool is_space(char x) {
    return x == ' ';
}

bool is_sq_bracket_open(char x) {
    return x == '[';
}

bool is_sq_bracket_close(char x) {
    return x == ']';
}

bool is_bracket_open(char x) {
    return x == '(';
}

bool is_bracket_close(char x) {
    return x == ')';
}

bool is_semicolon(char x) {
    return x == ';';
}

bool is_comma(char x) {
    return x == ',';
}

bool is_digit(char x) {
    return x >= '0' && x <= '9';
}

bool is_underscore(char x) {
    return x == '_';
}

bool is_eE(char x) {
    return (x == 'e' || x == 'E');
}

bool is_dollar(char x) {
    return (x == '$');
}

static const struct {
    const char *name;
    is_valid_char func;
} lookup_table[] = {
        {"is_small_alpha",      is_small_alpha},
        {"is_capital_alpha",    is_capital_alpha},
        {"is_alpha",            is_alpha},
        {"is_plus",             is_plus},
        {"is_minus",            is_minus},
        {"is_forward_slash",    is_forward_slash},
        {"is_greater_than",     is_greater_than},
        {"is_less_than",        is_less_than},
        {"is_star",             is_star},
        {"is_colon",            is_colon},
        {"is_equal",            is_equal},
        {"is_exclamation",      is_exclamation},
        {"is_dot",              is_dot},
        {"is_new_line",         is_new_line},
        {"is_tab",              is_tab},
        {"is_space",            is_space},
        {"is_sq_bracket_open",  is_sq_bracket_open},
        {"is_sq_bracket_close", is_sq_bracket_close},
        {"is_bracket_open",     is_bracket_open},
        {"is_bracket_close",    is_bracket_close},
        {"is_semicolon",        is_semicolon},
        {"is_comma",            is_comma},
        {"is_digit",            is_digit},
        {"is_underscore",       is_underscore},
        {"is_eE",               is_eE},
        {"is_dollar",           is_dollar}
};

// Define a function that looks up a function pointer by name
is_valid_char string_to_function(const char *str) {
    int lookup_len = sizeof(lookup_table) / sizeof(lookup_table[0]);
    for (int i = 0; i < lookup_len; i++) {
        if (strcmp(str, lookup_table[i].name) == 0) {
            return lookup_table[i].func;
        }
    }
    // Handle error (e.g., function name not found)
    return NULL;
}

const char *get_function_name(is_valid_char func) {
    int lookup_len = sizeof(lookup_table) / sizeof(lookup_table[0]);
    for (int i = 0; i < lookup_len; i++) {
        if (lookup_table[i].func == func) {
            return lookup_table[i].name;
        }
    }
    return NULL;
}
