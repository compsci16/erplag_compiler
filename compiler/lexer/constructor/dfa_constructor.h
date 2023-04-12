#ifndef COMPILER_DFA_CONSTRUCTOR_H
#define COMPILER_DFA_CONSTRUCTOR_H

#include "dfa.h"

DFA *get_initialized_dfa_from(const char *edges_path, const char *const accept_retract_other_path,
                              const char *id_to_token_type_path, const char *state_to_errors_path);


#endif //COMPILER_DFA_CONSTRUCTOR_H
