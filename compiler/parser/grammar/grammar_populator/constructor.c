#include <stdlib.h>
#include <assert.h>
#include "constructor.h"
#include "string_def.h"
#include "hashmap.h"
#include "grammar_constants.h"
#include "utility.h"
#include "int.h"
#include "filehandler.h"
#include "grammar/first_follow/first.h"
#include "grammar/first_follow/follow.h"
#include "core/parse_table/parse_table.h"

#define MAX_SYMBOLS_ON_RHS 24

static void
initialize_productions(char *path, Grammar *g);

static void initialize_string_to_non_terminal_map(Grammar *grammar);

static void initialize_string_to_action_map(Grammar *grammar);

static void initialize_string_to_terminal_map(Grammar *grammar);

const Grammar *get_initialized_grammar(char *productions_path) {
    Grammar *g = malloc_safe(sizeof(Grammar));
    initialize_string_to_terminal_map(g);
    initialize_string_to_non_terminal_map(g);
    initialize_string_to_action_map(g);
    initialize_productions(productions_path, g);

//    for (int i = 0; i < g->N_PRODUCTIONS; ++i) {
//        print_production(g, g->productions[i]);
//    }
    set_first_sets_for(g);
    set_follow_sets_for(g);
    Production ***table = get_parse_table_for(g);
    print_parse_table(g, table, "./parse_table.csv");
    return g;
}

static void initialize_string_to_terminal_map(Grammar *grammar) {
    HashMap *string_to_terminal_map = create_hashmap(sizeof(char *), sizeof(int), hash_string_djb2,
                                                     comparator_string);
    for (int i = TERMINALS_BEGIN; i <= TERMINALS_END; ++i) {
        put_in_hashmap(string_to_terminal_map, &SYMBOL_TO_STRING[i], &i);
    }
    grammar->string_to_terminal_map = string_to_terminal_map;
    // TODO: COMMENT PRINT
    // print_all_key_val_pairs_hashmap(string_to_terminal_map, print_string, print_int);
}

static void initialize_string_to_action_map(Grammar *grammar) {
    HashMap *string_to_action_map = create_hashmap(sizeof(char *), sizeof(int), hash_string_djb2,
                                                   comparator_string);
    for (int i = ACTIONS_BEGIN; i <= ACTIONS_END; ++i) {
        put_in_hashmap(string_to_action_map, &SYMBOL_TO_STRING[i], &i);
    }
    grammar->string_to_action_map = string_to_action_map;
    // TODO: COMMENT
    //   print_all_key_val_pairs_hashmap(string_to_action_map, print_string, print_int);
}


static void initialize_string_to_non_terminal_map(Grammar *grammar) {
    HashMap *string_to_non_terminal_map = create_hashmap(sizeof(char *), sizeof(int), hash_string_djb2,
                                                         comparator_string);
    for (int i = NON_TERMINALS_BEGIN; i <= NON_TERMINALS_END; ++i) {
        put_in_hashmap(string_to_non_terminal_map, &SYMBOL_TO_STRING[i], &i);
    }
    grammar->string_to_non_terminal_map = string_to_non_terminal_map;
    // TODO: COMMENT
    // print_all_key_val_pairs_hashmap(string_to_non_terminal_map, print_string, print_int);
}

static void initialize_productions(char *path, Grammar *g) {
    FILE *fp = fopen(path, "r");
    if (!fp) { report_file_error(path); }
    int BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];
    fgets(buffer, BUFFER_SIZE, fp);
    int N_PRODUCTIONS = atoi(buffer);
    const HashMap *str_to_non_terminal_map = g->string_to_non_terminal_map;
    const HashMap *str_to_terminal_map = g->string_to_terminal_map;
    const HashMap *str_to_action_map = g->string_to_action_map;
    Production **productions = malloc_safe(sizeof(Production *) * N_PRODUCTIONS);
    int *num_in_split = malloc_safe(sizeof(int));
    for (int i = 0; i < N_PRODUCTIONS; ++i) {
        fgets(buffer, BUFFER_SIZE, fp);
        // split by equal to separate LHS and RHS of production
        int max_strings_in_result = 2;
        char **arrow_split = split_by(buffer, "= ", num_in_split, max_strings_in_result);
        assert(*num_in_split == 2);
        char *LHS = arrow_split[0];
        char *RHS = arrow_split[1];
        // split RHS by comma
        int *non_terminal_LHS = get_from_hashmap(str_to_non_terminal_map, &LHS);
        if (non_terminal_LHS == NULL) {
            fprintf(stderr, "Did not find LHS = %s in str_to_non_terminal map", LHS);
            return;
        }
        max_strings_in_result = MAX_SYMBOLS_ON_RHS;
        char **rhs_split = split_by(RHS, ",\n\r ", num_in_split, max_strings_in_result);

        size_t production_size = sizeof(int) * (*num_in_split) + sizeof(Production);
        Production *production = malloc_safe(production_size);
        // LAST comma separated value is action number
        production->lhs = *non_terminal_LHS;
        int prod_rhs_size = 0;
        // printf("Production RHS Size = %d\n", production->rhs_size);
        for (int j = 0; j < *num_in_split; ++j) {
            char *rhs_j = rhs_split[j];
            if (hashmap_contains(str_to_terminal_map, &rhs_j)) {
                int *terminal = get_from_hashmap(str_to_terminal_map, &rhs_j);
                production->rhs[j] = *terminal;
                ++prod_rhs_size;
            } else if (hashmap_contains(str_to_non_terminal_map, &rhs_j)) {
                int *non_terminal = get_from_hashmap(str_to_non_terminal_map, &rhs_j);
                production->rhs[j] = *non_terminal;
                ++prod_rhs_size;
            } else if (hashmap_contains(str_to_action_map, &rhs_j)) {
                int *action = get_from_hashmap(str_to_action_map, &rhs_j);
                production->rhs[j] = *action;
                ++prod_rhs_size;
            } else {
                // TODO: Uncomment
                // fprintf(stderr, "Couldn't find %s in terminals or non terminals or actions", rhs_j);
            }
        }
        production->rhs_size = prod_rhs_size;
        productions[i] = production;
    }
    g->productions = productions;
    g->N_PRODUCTIONS = N_PRODUCTIONS;
    free(num_in_split);
    fclose(fp);
}
