#include <stdlib.h>
#include "first.h"
#include "utility/int.h"
#include "grammar/cfg.h"
#include "grammar/grammar_populator/grammar_constants.h"
#include <string.h>
#include "action_utility.h"

static HashMap *get_map_with_initialized_sets_for_both_terminals_non_terminals(Grammar *g);

static void first_set_helper(const Grammar *g, int symbols[], int size, int index, HashSet *current_first_set);

static void initialize_terminal_first_sets(HashMap *map, Grammar *g);

error set_first_sets_for(Grammar *g) {
    // contains first sets for both terminals and non-terminals (symbols)
    HashMap *symbol_to_first_map = get_map_with_initialized_sets_for_both_terminals_non_terminals(g);
    initialize_terminal_first_sets(symbol_to_first_map, g);
    bool first_sets_are_changing = true;
    // to determine if first sets are in fact changing
    size_t sizes_non_terminals_first_sets[N_NON_TERMINALS + N_TERMINALS];
    memset(sizes_non_terminals_first_sets, 0, sizeof(size_t) * N_NON_TERMINALS);
    int N_PRODUCTIONS = g->N_PRODUCTIONS;

    // O( # NT )
    for (int i = NON_TERMINALS_BEGIN; i <= NON_TERMINALS_END; ++i) {
        HashSet **nt_hashset = get_from_hashmap(symbol_to_first_map, &i);
        sizes_non_terminals_first_sets[i] = get_size_hashset(*nt_hashset);
    }

    int epsilon = EPSILON;
    while (first_sets_are_changing) {
        // O(TP + N)
        for (int p = 0; p < N_PRODUCTIONS; ++p) {
            Production *current_production = g->productions[p];
            int last_rhs_position = get_last_rhs_position(current_production);
            HashSet *temp = create_hashset(sizeof(int), hash_int, comparator_int);
            // ignore actions to go to B0

            int rhs_0_position = get_next_rhs_i_position_from_and_including(current_production, 0);

            // temp gets first(rhs[0]) as that is going to be in first of lhs non-terminal
            HashSet **rhs_0_hashset = get_from_hashmap(symbol_to_first_map,
                                                       &current_production->rhs[rhs_0_position]);
            error e = hashset_union_with(temp, *rhs_0_hashset); // O(T)

            if (e != ERROR_OK) {
                fprintf(stderr, "Error Merging temp");
                destroy_hashset(temp);
                destroy_hashmap(symbol_to_first_map);
                return ERROR_CREATING_FIRST_SET;
            }


            // to check if others can also lead to empty
            remove_from_hashset(temp, &epsilon);

            int i = rhs_0_position;
            int second_last_rhs_position = get_second_last_rhs_position(current_production);
            while (i <= second_last_rhs_position) {
                // ignore actions while processing to create parse table
                if (is_action(current_production->rhs[i])) {
                    ++i;
                    continue;
                }

                HashSet **rhs_i_hashset = get_from_hashmap(symbol_to_first_map, &current_production->rhs[i]);
                if (!hashset_contains(*rhs_i_hashset, &epsilon)) {
                    break;
                }
                int rhs_next_i = get_next_rhs_i_position_from_and_including(current_production, i + 1);
                HashSet **rhs_next_i_hashset = get_from_hashmap(symbol_to_first_map,
                                                                &current_production->rhs[rhs_next_i]);
                HashSet *next_set = clone(*rhs_next_i_hashset);
                remove_from_hashset(next_set, &epsilon);
                e = hashset_union_with(temp, next_set);

                if (e != ERROR_OK) {
                    fprintf(stderr, "Error Merging temp");
                    destroy_hashset(temp);
                    destroy_hashset(next_set);
                    destroy_hashmap(symbol_to_first_map);
                    free(temp);
                    free(next_set);
                    free(symbol_to_first_map);
                    return ERROR_CREATING_FIRST_SET;
                }
                i = rhs_next_i;
                destroy_hashset(next_set);
            }

            if (i == last_rhs_position) {
                HashSet **rhs_last_hashset = get_from_hashmap(symbol_to_first_map,
                                                              &current_production->rhs[last_rhs_position]);
                if (hashset_contains(*rhs_last_hashset, &epsilon)) { add_to_hashset(temp, &epsilon); }
            }

            HashSet **first_set_LHS = get_from_hashmap(symbol_to_first_map, &current_production->lhs);
            add_all_from_hashset(temp, *first_set_LHS);
            destroy_hashset(temp);
        }


        bool did_size_of_any_NT_set_change = false;
        for (int i = NON_TERMINALS_BEGIN; i <= NON_TERMINALS_END; ++i) {
            HashSet **nt_set = get_from_hashmap(symbol_to_first_map, &i);
            size_t new_size = get_size_hashset(*nt_set);
            if (new_size != sizes_non_terminals_first_sets[i]) {
                did_size_of_any_NT_set_change = true;
            }
            sizes_non_terminals_first_sets[i] = new_size;
        }
        first_sets_are_changing = did_size_of_any_NT_set_change;
    }
    g->first_sets_map = symbol_to_first_map;
    return ERROR_OK;
}

// Finds first set of a production
HashSet *first_set_of_symbols(const Grammar *g, int symbols[], int size) {
    HashSet *first_set = create_hashset(sizeof(int), hash_int, comparator_int);
    int first_rhs_pos = 0;
    while (is_action(symbols[first_rhs_pos])) { ++first_rhs_pos; }
    first_set_helper(g, symbols, size, first_rhs_pos, first_set);
    return first_set;
}

// Todo: Check algorithm
static void first_set_helper(const Grammar *g, int symbols[], int size, int index, HashSet *current_first_set) {
    if (index >= size) {
        return;
    }
    if (is_action(symbols[index])) {
        first_set_helper(g, symbols, size, index + 1, current_first_set);
    } else if (is_terminal(symbols[index])) {
        add_to_hashset(current_first_set, &symbols[index]);
        return;
    } else if (is_non_terminal(symbols[index])) {
        HashMap *first_sets_map = g->first_sets_map;
        int epsilon = EPSILON;
        HashSet **first_set_of_current = get_from_hashmap(first_sets_map, &symbols[index]);
        add_all_from_hashset(*first_set_of_current, current_first_set);
        if (index < size - 1 && hashset_contains(*first_set_of_current, &epsilon)) {
            first_set_helper(g, symbols, size, index + 1, current_first_set);
            return;
        }
    } else {
        fprintf(stderr, "Undefined symbol");
    }

}

// Initializes hashset for terminals and non-terminals with empty hashsets
static HashMap *get_map_with_initialized_sets_for_both_terminals_non_terminals(Grammar *g) {
    // where symbol is int (i.e. Either the Terminal or the Non Terminal Enum
    HashMap *symbol_to_first_map = create_hashmap(sizeof(int), sizeof(struct HashSet *),
                                                  hash_int, comparator_int);
    for (int i = TERMINALS_BEGIN; i <= TERMINALS_END; i++) {
        HashSet *hs = create_hashset(sizeof(int), hash_int, comparator_int);
        put_in_hashmap(symbol_to_first_map, &i, &hs);
    }

    for (int i = NON_TERMINALS_BEGIN; i <= NON_TERMINALS_END; i++) {
        HashSet *hs = create_hashset(sizeof(int), hash_int, comparator_int);
        put_in_hashmap(symbol_to_first_map, &i, &hs);
    }

    return symbol_to_first_map;
}

// Initializes terminal's first sets of terminals FirstSet(Terminal) = {Terminal}
static void initialize_terminal_first_sets(HashMap *map, Grammar *g) {
    for (int i = TERMINALS_BEGIN; i <= TERMINALS_END; ++i) {
        HashSet **first_set_for_terminal = get_from_hashmap(map, &i);
        add_to_hashset(*first_set_for_terminal, &i);
    }
}