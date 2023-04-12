#include <string.h>
#include "cfg.h"
#include "utility/int.h"
#include "utility/utility.h"
#include "grammar/grammar_populator/grammar_constants.h"

void print_symbol(const void *symbol) {
    const int *x = symbol;
    int val = *x;
    if (is_terminal(val) || is_non_terminal(val) || is_action(val)) {
        printf("%s", SYMBOL_TO_STRING[val]);
    } else {
        printf("Undefined symbol");
    }
}


void print_production(const Grammar *g, Production *p) {
    print_symbol(&p->lhs);
    printf(" --> ");
    for (int i = 0; i < p->rhs_size; ++i) {
        int s = p->rhs[i];
        print_symbol(&s);
        printf(" ");
    }
    puts("");
}

char *get_production_string(Grammar *g, Production *p) {
    int MAX_PRODUCTION_SIZE = (MAX_TERMINAL_SIZE + MAX_NON_TERMINAL_SIZE) * 10;
    char *string = calloc_safe(MAX_PRODUCTION_SIZE, sizeof(char));
    strcat(string, SYMBOL_TO_STRING[p->lhs]);
    strcat(string, " -> ");
    for (int i = 0; i < p->rhs_size; ++i) {
        int symbol = p->rhs[i];
        if (is_terminal(symbol) || is_non_terminal(symbol) || is_action(symbol)) {
            strcat(string, SYMBOL_TO_STRING[symbol]);
        } else {
            strcat(string, "unknown");
        }
        strcat(string, " ");
    }
    return string;
}

bool is_null_production(Production *p) {
    bool has_epsilon_on_rhs = false;
    int rhs_size = 0;
    for (int i = 0; i < p->rhs_size; ++i) {
        if (is_terminal(p->rhs[i]) || is_non_terminal(p->rhs[i])) {
            ++rhs_size;
            if (p->rhs[i] == EPSILON) {
                has_epsilon_on_rhs = true;
            }
        }
    }
    return rhs_size == 1 && has_epsilon_on_rhs;
}