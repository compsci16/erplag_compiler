#include "follow.h"
#include "utility/int.h"
#include "grammar/grammar_populator/grammar_constants.h"
#include <string.h>
#include "action_utility.h"

static HashMap *get_initialized_non_terminal_follow_sets_with_empty_sets();

// assumes grammar with first sets initialized is passed
error set_follow_sets_for(Grammar *g) {
    HashMap *follow_map = get_initialized_non_terminal_follow_sets_with_empty_sets(g);
    int START_SYMBOL = Pro;
    int DOLLAR_SYMBOL = DOLLAR;
    HashSet **start_set = get_from_hashmap(follow_map, &START_SYMBOL);
    add_to_hashset(*start_set, &DOLLAR_SYMBOL);
    int epsilon = EPSILON;

    bool follow_sets_are_still_changing = true;

    size_t follow_sets_sizes[ZIdList + N_TERMINALS];
    memset(follow_sets_sizes, 0, N_NON_TERMINALS * sizeof(size_t));

    while (follow_sets_are_still_changing) {
        for (int i = 0; i < g->N_PRODUCTIONS; ++i) {
            // Consider A -> B0 B1 B2 ... Bk from right to left
            Production *current_production = g->productions[i];
            int lhs = current_production->lhs;
            HashSet **trailer_required = get_from_hashmap(follow_map, &lhs);
            HashSet *trailer = clone(*trailer_required);
            int last_rhs_position = get_last_rhs_position(current_production);
            for (int j = last_rhs_position; j >= 0; --j) {
                int bj = current_production->rhs[j];
                if (is_non_terminal(bj)) {
                    HashSet **follow_set_bj = get_from_hashmap(follow_map, &bj);
                    add_all_from_hashset(trailer, *follow_set_bj);

                    HashSet **first_set_bj = get_from_hashmap(g->first_sets_map, &bj);
                    if (hashset_contains(*first_set_bj, &epsilon)) {
                        add_all_from_hashset(*first_set_bj, trailer);
                        remove_from_hashset(trailer, &epsilon);
                    } else {
                        trailer_required = get_from_hashmap(g->first_sets_map, &bj);
                        destroy_hashset(trailer);
                        trailer = clone(*trailer_required);
                    }
                } else if (is_terminal(bj)) {
                    destroy_hashset(trailer);
                    trailer = create_hashset(sizeof(int), hash_int, comparator_int);
                    add_to_hashset(trailer, &bj);
                }
            }
            destroy_hashset(trailer);

        }

        bool did_any_size_change = false;
        for (int i = NON_TERMINALS_BEGIN; i <= NON_TERMINALS_END; ++i) {
            HashSet **hs = get_from_hashmap(follow_map, &i);
            size_t current_size = get_size_hashset(*hs);
            if (!did_any_size_change && current_size != follow_sets_sizes[i]) {
                did_any_size_change = true;
            }
            follow_sets_sizes[i] = current_size;
        }
        follow_sets_are_still_changing = did_any_size_change;
    }
    g->follow_sets_map = follow_map;
    for (int i = NON_TERMINALS_BEGIN; i <= NON_TERMINALS_END; ++i) {
        // printf("Follow of: %s\n", SYMBOL_TO_STRING[i]);
        HashSet **curr = get_from_hashmap(follow_map, &i);
        // print_all_keys_hashset(*curr, print_symbol);
    }
    return ERROR_OK;
}


static HashMap *get_initialized_non_terminal_follow_sets_with_empty_sets() {
    HashMap *follow_map = create_hashmap(sizeof(int), sizeof(HashSet *), hash_int, comparator_int);
    for (int i = NON_TERMINALS_BEGIN; i <= NON_TERMINALS_END; ++i) {
        HashSet *empty = create_hashset(sizeof(int), hash_int, comparator_int);
        put_in_hashmap(follow_map, &i, &empty);
    }
    return follow_map;
}
