#include "action_utility.h"
#include "grammar/grammar_populator/grammar_constants.h"

int get_second_last_rhs_position(Production *production) {
    int last_pos = get_last_rhs_position(production) - 1;
    while (is_action(production->rhs[last_pos])) {
        --last_pos;
    }
    return last_pos;
}

int get_last_rhs_position(Production *production) {
    int last_rhs_position = production->rhs_size - 1;
    while (is_action(production->rhs[last_rhs_position])) {
        --last_rhs_position;
    }
    return last_rhs_position;
}

int get_next_rhs_i_position_from_and_including(Production *production, int i) {
    while (is_action(production->rhs[i])) {
        ++i;
    }
    return i;
}
