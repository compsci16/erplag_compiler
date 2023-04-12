#ifndef COMPILER_DFA_H
#define COMPILER_DFA_H

#include "datastructures/arraylist/arraylist.h"
#include "datastructures/hashmap/hashmap.h"
#include "checkers.h"
#include "lexer.h"

#define IDENTIFIER_STATE_ID_VAL 2

typedef struct State {
    int id;
    ArrayList *edges;
    struct State *other;
    bool is_accept;
    int retract_by;
    int token_associated;
} State;

typedef struct Edge {
    is_valid_char fn;
    int next_id;
} Edge;

typedef struct DFA {
    State *start;
    int current_state_id;
    int N_STATES;
    int ERROR_STATE_ID;
    int IDENTIFIER_STATE_ID;
    char **state_to_error;
    State states[];
} DFA;

void invoke(DFA *dfa, char on);

void print_dfa_for(Lexer *lexer, FILE *fp);

#endif //COMPILER_DFA_H
