#include "grammar/grammar_populator/grammar_constants.h"
#include "lexer.h"
#include "dfa.h"
#include "checkers.h"

static void print_state(Lexer *lexer, State s, FILE *fp);

static void print_sep_line(FILE *fp);

void print_dfa_for(Lexer *lexer, FILE *fp) {
    DFA *dfa = lexer->dfa;
    fprintf(fp, "Number of states = %d\n", dfa->N_STATES);
    for (int i = 0; i < dfa->N_STATES; ++i) {
        print_state(lexer, dfa->states[i], fp);
    }
}

void invoke(DFA *dfa, char on) {
    State current = dfa->states[dfa->current_state_id];
    size_t num_edges = get_arraylist_size(current.edges);
    for (size_t i = 0; i < num_edges; ++i) {
        const Edge *e = get_element_at_index_arraylist(current.edges, i);
        is_valid_char fn = e->fn;
        if (fn(on)) {
            // Found a valid edge, update current state by transition
            dfa->current_state_id = e->next_id;
            return;
        }
    }
    // Did not find any matching transition, check if others, otherwise send to trap state
    if (current.other != NULL) {
        dfa->current_state_id = current.other->id;
        return;
    }
    dfa->current_state_id = dfa->ERROR_STATE_ID;
}

static void print_state(Lexer *lexer, State s, FILE *fp) {
    print_sep_line(fp);
    size_t number_edges = get_arraylist_size(s.edges);
    char *is_accept = s.is_accept ? "true" : "false";
    fprintf(fp, "State ID = %d \nis_accept = %s\nretract_by = %d\n# edges = %zu\n", s.id, is_accept, s.retract_by,
            number_edges);

    fprintf(fp, "Associated Token: %s\n", SYMBOL_TO_STRING[s.token_associated]);

    fprintf(fp, "Edges:\n");
    if (s.other != NULL) { fprintf(fp, "on others to %d\n", s.other->id); }
    else { fprintf(fp, "on others to NOWHERE\n"); }
    for (int i = 0; i < number_edges; ++i) {
        const Edge *e = get_element_at_index_arraylist(s.edges, i);
        const char *name = get_function_name(e->fn);
        fprintf(fp, "on %s to %d,\n", name, e->next_id);
    }
    print_sep_line(fp);
}

static void print_sep_line(FILE *fp) {
    for (int i = 0; i < 50; ++i) {
        fprintf(fp, "-");
    }
    fprintf(fp, "\n");
}
