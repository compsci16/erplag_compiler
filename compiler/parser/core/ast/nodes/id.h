#ifndef COMPILER_ID_H
#define COMPILER_ID_H

#include "core/ast/ast.h"

IDNode *create_id_node();

IDNode *create_id_node_with_token(Token *token);

#endif //COMPILER_ID_H
