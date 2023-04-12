#include "dfa_constructor.h"
#include "utility/filehandler.h"
#include "utility/utility.h"
#include "utility/errors.h"
#include "dfa.h"
#include "checkers.h"
#include <stdio.h>

static error initialize_edges(const char *edges_path, DFA *dfa);

static DFA *get_dfa_with_initialized_states(const char *states_path);

static void initialize_accept_retract_other(const char *path, DFA *dfa);

static void initialize_states(DFA *dfa);

static void initialize_state_to_error_message(const char *state_to_errors_path, DFA *dfa);

DFA *get_initialized_dfa_from(const char *edges_path, const char *const accept_retract_other_path,
                              const char *id_to_token_type_path, const char *state_to_errors_path) {
    DFA *dfa = get_dfa_with_initialized_states(edges_path);
    if (dfa != NULL) {
        initialize_edges(edges_path, dfa);
        initialize_accept_retract_other(accept_retract_other_path, dfa);
        initialize_state_to_error_message(state_to_errors_path, dfa);
        dfa->start = &dfa->states[0];
        dfa->current_state_id = dfa->start->id;
        dfa->ERROR_STATE_ID = dfa->states[dfa->N_STATES - 1].id;
        dfa->IDENTIFIER_STATE_ID = IDENTIFIER_STATE_ID_VAL;
    }
    return dfa;
}

static void initialize_state_to_error_message(const char *state_to_errors_path, DFA *dfa) {
    FILE *fp = fopen(state_to_errors_path, "r");
    if (!fp) {
        report_file_error(state_to_errors_path);
    }
    char **state_to_err = calloc_safe((size_t) dfa->N_STATES, sizeof(char *));

    int BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];
    for (int i = 0; i < dfa->N_STATES; ++i) {
        fgets(buffer, BUFFER_SIZE, fp);
        buffer[strcspn(buffer, "\n\r")] = 0;
        state_to_err[i] = strdup(buffer);
    }
    dfa->state_to_error = state_to_err;
    fclose(fp);
}


static void initialize_accept_retract_other(const char *path, DFA *dfa) {
    FILE *fp = fopen(path, "r");
    if (!fp) { report_file_error(path); }
    int BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];
    fgets(buffer, BUFFER_SIZE, fp); // ignore 1st line
    State *states = dfa->states;
    for (int i = 0; i < dfa->N_STATES; ++i) {
        fgets(buffer, BUFFER_SIZE, fp);
        buffer[strcspn(buffer, "\n\r")] = 0;
        int size_split;
        char **split = split_by(buffer, ",", &size_split, 4);
        if (size_split != 4) {
            fprintf(stderr, "Split Size = %d, Line # = %d,Invalid line format", size_split, (i + 1));
            fclose(fp);
            return;
        }
        states[i].id = i;
        int is_accept = atoi(split[1]);
        states[i].is_accept = is_accept == 1 ? true : false;
        states[i].retract_by = atoi(split[2]);
        int other_state = atoi(split[3]);
        states[i].other = other_state == -1 ? NULL : &states[other_state];
    }


    fclose(fp);
}

static DFA *get_dfa_with_initialized_states(const char *states_path) {
    FILE *fp = fopen(states_path, "r");
    if (!fp) {
        report_file_error(states_path);
        return NULL;
    }

    const int BUFFER_SIZE = 16;
    char buffer[BUFFER_SIZE];
    fgets(buffer, BUFFER_SIZE, fp);
    const int N_STATES = atoi(buffer);
    size_t dfa_size = sizeof(struct DFA) + sizeof(struct State) * (unsigned long) N_STATES;
    DFA *dfa = malloc_safe(dfa_size);
    dfa->N_STATES = N_STATES;
    initialize_states(dfa);

    fclose(fp);
    return dfa;
}

static void initialize_states(DFA *dfa) {
    for (int i = 0; i < dfa->N_STATES; ++i) {
        dfa->states[i].edges = create_arraylist(sizeof(struct Edge));
    }
}

static error initialize_edges(const char *edges_path, DFA *dfa) {
    FILE *fp = fopen(edges_path, "r");
    if (!fp) {
        report_file_error(edges_path);
        return ERROR_READING_FILE;
    }
    const int BUFFER_SIZE = 1024;
    const int MAX_EDGES = 25;
    char buffer[BUFFER_SIZE];
    fgets(buffer, BUFFER_SIZE, fp); // ignore first line
    for (int i = 0; i < dfa->N_STATES; ++i) {
        fgets(buffer, BUFFER_SIZE, fp);
        buffer[strcspn(buffer, "\n\r")] = '\0';
//        printf("Buffer Before: %s\n", buffer);
        int split;
        char **split_initial = split_by(buffer, ": ", &split, 2);
        if (split != 2) {
            continue;
        }
        assert(split == 2);
        //printf("Splitting for state: %s\n", split_initial[0]);
//        printf("Buffer After: %s\n", buffer);
        char *edges_all = split_initial[1];
        //  printf("Edges all:%s\n", edges_all);
        char **edges_list = split_by(edges_all, ",", &split, MAX_EDGES);
        //printf("Number of edges for %d = %d edges\n", i, split);
        ArrayList *edges = dfa->states[i].edges;
        for (int j = 0; j < split; ++j) {
            int x;
            char **split_edge_data = split_by(edges_list[j], "=", &x, 2);
            assert(x == 2);
//            printf("%s\n", split_edge_data[0]);
//            printf("%s\n", split_edge_data[1]);
            is_valid_char fn = string_to_function(split_edge_data[0]);
            int next_state = atoi(split_edge_data[1]);
            struct Edge edge = {fn, next_state};
            add_element_arraylist(edges, &edge);
        }

        free(split_initial[0]);
        free(split_initial[1]);
        for (int j = 0; j < split; ++j) {
            free(edges_list[j]);
        }
        free(edges_list);
    }
    fclose(fp);
    return ERROR_OK;
}

