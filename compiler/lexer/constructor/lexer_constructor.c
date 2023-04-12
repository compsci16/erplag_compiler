#include "lexer_constructor.h"
#include "utility/filehandler.h"
#include "utility/utility.h"
#include "utility/string_def.h"
#include "dfa_constructor.h"
#include "grammar/grammar_populator/grammar_constants.h"
#include "int.h"
#include "lexer.h"
#include "dfa.h"

static void mark_accept_states_of_dfa_with_tokens(Lexer *lexer, char *path);

static void initialize_lexer_file_fields(Lexer *lexer, char *source_code);

static void initialize_state_id_to_token_map_for_dfa(Lexer *lexer, const char *id_to_token_type_path);

static void set_lexer_buffer_size_and_initialize_buffers(Lexer *lexer, int size);

static void set_lexer_buffer_size_and_initialize_buffers(Lexer *lexer, int size) {
    lexer->buffer_size = size;
    lexer->buffer1 = calloc_safe((size_t) size, sizeof(char));
    lexer->buffer2 = calloc_safe((size_t) size, sizeof(char));
}

Lexer *
get_initialized_lexer_for_parser(Parser *parser, char *source_code_path, char *state_to_token_path,
                                 const char *edges_path,
                                 char *accept_retract_other_path, const char *state_id_to_token_path, const char *
state_to_error_path, int size) {
    Lexer *lexer = malloc_safe(sizeof(Lexer));
    lexer->returned_dollar_token = false;
    lexer->string_to_token_type_map = parser->grammar->string_to_terminal_map;
    lexer->forward = 0;
    lexer->dfa = get_initialized_dfa_from(edges_path, accept_retract_other_path, state_id_to_token_path,
                                          state_to_error_path);
    lexer->current_token = NULL;
    mark_accept_states_of_dfa_with_tokens(lexer, state_to_token_path);
    set_lexer_buffer_size_and_initialize_buffers(lexer, size);
    initialize_lexer_file_fields(lexer, source_code_path);
    initialize_state_id_to_token_map_for_dfa(lexer, state_id_to_token_path);
    return lexer;
}


static void initialize_state_id_to_token_map_for_dfa(Lexer *lexer, const char *id_to_token_type_path) {
    FILE *fp = fopen(id_to_token_type_path, "r");
    if (!fp) { report_file_error(id_to_token_type_path); }
    HashMap *string_to_token_map = lexer->string_to_token_type_map;
    int BUF_SIZE = MAX_TOKEN_SIZE;
    char *buffer = calloc_safe((size_t) BUF_SIZE, sizeof(char));
    DFA *dfa = lexer->dfa;
    for (int i = 0; i < dfa->N_STATES; ++i) {
        fgets(buffer, BUF_SIZE, fp);
        buffer[strcspn(buffer, "\r\n")] = 0;
        int *token = get_from_hashmap(string_to_token_map, &buffer);
        if (token == NULL) {
            fprintf(stderr, "Invalid token type %s\n", buffer);
            exit(1);
        }
        dfa->states[i].token_associated = *token;
    }
    free(buffer);
}

static void initialize_lexer_file_fields(Lexer *lexer, char *source_code) {
    FILE *fp = fopen(source_code, "r");
    if (!fp) {
        report_file_error(source_code);
        return;
    }
    lexer->file = fp;
    lexer->current_line = 1;
    lexer->buffer_number = 1;
    lexer->buffer1[0] = '$';
    lexer->buffer2[0] = '$';
    lexer->file_name = strdup(source_code);
}

static void mark_accept_states_of_dfa_with_tokens(Lexer *lexer, char *path) {
    FILE *fp = fopen(path, "r");
    if (!fp) {
        report_file_error(path);
        return;
    }
    struct DFA *dfa = lexer->dfa;
    State *states = dfa->states;
    HashMap *string_to_token_map = lexer->string_to_token_type_map;
    int BUF_SIZE = 36;
    char *buffer = calloc_safe(BUF_SIZE, sizeof(char));
    // TODO: REMOVE:
    //   print_all_key_val_pairs_hashmap(lexer->string_to_token_type_map, print_string, print_int);
    for (int i = 0; i < dfa->N_STATES; ++i) {
        fgets(buffer, BUF_SIZE, fp);
        buffer[strcspn(buffer, "\r\n")] = 0;
        int *token_val = get_from_hashmap(string_to_token_map, &buffer);
        states[i].token_associated = *token_val;
    }
}


