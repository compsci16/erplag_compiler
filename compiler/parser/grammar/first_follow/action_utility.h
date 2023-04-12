#ifndef COMPILER_ACTION_UTILITY_H
#define COMPILER_ACTION_UTILITY_H

#include "grammar/cfg.h"

int get_next_rhs_i_position_from_and_including(Production *production, int i);

int get_second_last_rhs_position(Production *production);

int get_last_rhs_position(Production *production);

#endif //COMPILER_ACTION_UTILITY_H
