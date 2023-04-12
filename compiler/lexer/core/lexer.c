#include <stdio.h>
#include <string.h>
#include "token.h"
#include "utility/utility.h"
#include "utility/filehandler.h"
#include "grammar/grammar_populator/grammar_constants.h"
#include "lexer.h"
#include "dfa.h"
#include "compiler/error_reporter/error_report.h"

static void reload_buffer(Lexer *lexer, char *buffer);

static void reload_buffer_for(Lexer *lexer);

static void print_token_to(Lexer *lexer, Token *t, FILE *out);

static void close_lexer_file(Lexer *lexer);

static Token *get_dollar_token(const Lexer *lexer);

static Token *handle_eof(Lexer *lexer, char *lexeme, int lexeme_len);

static Token *handle_accept(Lexer *lexer, char *lexeme, int lexeme_len, int start_line);

static Token *create_token_for(const char *lexeme, int line_number, int token_type);

static Token *
handle_error_state(Lexer *lexer, char *lexeme, int lexeme_length, bool at_end, DFA *dfa, int previous_state);

void destroy_lexer(Lexer *lexer) {
    free(lexer->buffer1);
    free(lexer->buffer2);
    free(lexer->file_name);
    DFA *dfa = lexer->dfa;
    free(dfa->states);
    for (int i = 0; i < dfa->N_STATES; ++i) {
        destroy_arraylist(dfa->states[i].edges);
        free(dfa->state_to_error[i]);
    }
    free(dfa);
}

Token *get_next_token(Lexer *lexer) {
    if (lexer->returned_dollar_token) {
        // as dollar should be the last token returned, any further calls should return NULL
        return NULL;
    }
    if (lexer->file == NULL) {
        // dollar should be the last token returned after closing file
        lexer->returned_dollar_token = true;
        return get_dollar_token(lexer);
    }

    int current_max_allowed = MAX_LEXEME_LENGTH;
    // + 1 to allow null terminator
    char *lexeme = calloc_safe((MAX_LEXEME_LENGTH + 1), sizeof(char));
    int lexeme_len = 0;
    char *lexer_buffer = lexer->buffer_number == 1 ? lexer->buffer1 : lexer->buffer2;
    DFA *dfa = lexer->dfa;
    dfa->current_state_id = dfa->start->id;
    int started_at_line = lexer->current_line;
    while (true) {
        char on = lexer_buffer[lexer->forward];
        lexer->forward++;

        if (on == '\n') {
            /* replace new line by the equivalent " " so that line number is incremented exactly once irrespective of
             retraction */
            lexer_buffer[lexer->forward - 1] = ' ';
            lexer->current_line++;
        }

        if (on == DOLlAR) {
            reload_buffer_for(lexer);
            lexer_buffer = lexer->buffer_number == 1 ? lexer->buffer1 : lexer->buffer2;
            on = lexer_buffer[lexer->forward++];

            if (on == '\n') {
                /* replace new line by the equivalent " " so that line number is incremented exactly once irrespective of
                 retraction */
                lexer_buffer[lexer->forward - 1] = ' ';
                lexer->current_line++;
            }


            if (on == DOLlAR) {
                lexer->current_token = handle_eof(lexer, lexeme, lexeme_len);
                return lexer->current_token;
            }
        }
        lexeme[lexeme_len] = on;
        ++lexeme_len;
        if (lexeme_len == current_max_allowed) {
            /*
             * When? - Lexer is scanning an identifier with greater than MAX_LEXEME_LENGTH
             * Store the whole lexeme but also report it as an error later by checking lexeme length
             * corresponding to the final state = IDENTIFIER_STATE_ID
             */
            int new_current_max_allowed = current_max_allowed * 2 + 1;
            lexeme = realloc(lexeme, new_current_max_allowed);
            current_max_allowed = new_current_max_allowed;
        }

        //lexer->forward++;
        int previous_state = dfa->current_state_id;
        invoke(dfa, on);

        State current = dfa->states[dfa->current_state_id];
        if (current.is_accept) {
            lexer->current_token = handle_accept(lexer, lexeme, lexeme_len, started_at_line);
            return lexer->current_token;
        }
        if (current.id == dfa->ERROR_STATE_ID) {
            lexer->current_token = handle_error_state(lexer, lexeme, lexeme_len, false, dfa, previous_state);
            return lexer->current_token;
        }

    }
}

Token *get_current_token(Lexer *lexer) {
    return lexer->current_token;
}


static Token *handle_accept(Lexer *lexer, char *lexeme, int lexeme_len, int start_line) {
    DFA *dfa = lexer->dfa;
    int current_state_id = dfa->current_state_id;
    State current = dfa->states[current_state_id];
    lexer->forward -= current.retract_by;
    lexeme_len -= current.retract_by;
    lexeme[lexeme_len] = 0;

    if (current.token_associated == INVALID) {
        // DFA may reach the accept-state corresponding to the Invalid tokens "'\n', ' ', '\t'"
        free(lexeme);
        return get_next_token(lexer);
    }

//    Token *token = create_token_for(lexeme, lexer->current_line, current.token_associated);
    Token *token = create_token_for(lexeme, start_line, current.token_associated);

    if (current.id == dfa->IDENTIFIER_STATE_ID) {
        if (lexeme_len > MAX_LEXEME_LENGTH) {
            char error_message[256];
            sprintf(error_message, "%s %d. %s %d %s %s" "\n",
                    "Lexical Error on Line # =>", lexer->current_line, "Maximum Identifier Limit",
                    MAX_LEXEME_LENGTH,
                    "exceeded by identifier", token->lexeme);
            report_error(LEXICAL_ERROR, error_message);
            return token;
        }
//        SymbolTableEntry s = search_in_symbol_table(lexer->symbol_table, token->lexeme);
        SymbolTableEntry *s = search_in_symbol_table(lexer->symbol_table, token->lexeme);
        if (s != NULL) {
            char *token_type_string = get_upper(s->name);
            int *token_type = get_from_hashmap(lexer->string_to_token_type_map, &token_type_string);
            token->token_type = *token_type;
        }
    }
    return token;
}

static Token *handle_eof(Lexer *lexer, char *lexeme, int lexeme_len) {
    DFA *dfa = lexer->dfa;
    // Both buffers of the twin-buffer-pair begin at dollar => Reached EOF
    if (dfa->current_state_id == dfa->start->id) {
        // Start can't go anywhere on EOF, simply return Dollar token
        lexer->returned_dollar_token = true;
        return get_dollar_token(lexer);
    }
    int previous_state = dfa->current_state_id;
    /*
       * Invoke "others" on dfa's current state
       * Reached END of file, now let's say current state is just before ID's accept state
       * Now ID needs the "others" transition but it's not available as we reached EOF
       * So invoke others transition on that state to reach accept state
       *
       * Summary: Treat EOF (aka Sentinel aka DOLLAR) as "others"
     */
    invoke(dfa, '$');
    // Handle Error that occurs at the EOF
    if (dfa->current_state_id == dfa->ERROR_STATE_ID) {
        handle_error_state(lexer, lexeme, lexeme_len, true, dfa, previous_state);
        Token *d = get_dollar_token(lexer);
        close_lexer_file(lexer);
        lexer->returned_dollar_token = true;
        return d;
    }

    lexeme[lexeme_len] = 0;
    Token *token = create_token_for(lexeme, lexer->current_line,
                                    dfa->states[dfa->current_state_id].token_associated);

    if (dfa->current_state_id == dfa->IDENTIFIER_STATE_ID) {
        if (lexeme_len >= MAX_LEXEME_LENGTH) {
            char error_message[256];
            sprintf(error_message, "%s %d. %s %d %s %s" "\n",
                    "Lexical Error on Line # =>", lexer->current_line, "Maximum Identifier Limit",
                    MAX_LEXEME_LENGTH,
                    "exceeded by identifier", token->lexeme);
            report_error(LEXICAL_ERROR, error_message);
        } else {
            SymbolTableEntry *s = search_in_symbol_table(lexer->symbol_table, lexeme);
            if (s != NULL) {
                char *token_type_string = get_upper(s->name);
                int *token_type = get_from_hashmap(lexer->string_to_token_type_map, &token_type_string);
                token->token_type = *token_type;
            }
        }
    }
    if (dfa->states[dfa->current_state_id].token_associated == INVALID) {
        free(lexeme);
        free(token->lexeme);
        free(token);
        close_lexer_file(lexer);
        return get_next_token(lexer);
    }
    close_lexer_file(lexer);
    return token;
}

static void close_lexer_file(Lexer *lexer) {
    fclose(lexer->file);
    lexer->file = NULL;
}

// Handles error state and deallocates lexeme memory
static Token *
handle_error_state(Lexer *lexer, char *lexeme, int lexeme_length, bool at_end, DFA *dfa, int previous_state) {
    State *start = lexer->dfa->start;
    // check if symbol exists in alphabet
    ArrayList *edges = start->edges;
    size_t n_edges = get_arraylist_size(start->edges);
    for (size_t i = 0; i < n_edges; ++i) {
        Edge *e = get_element_at_index_arraylist(edges, i);
        is_valid_char fn = e->fn;
        if (fn(lexeme[lexeme_length - 1])) {
            lexer->forward -= 1;
            if (!at_end) { lexeme[lexeme_length - 1] = 0; }
            else { lexeme[lexeme_length] = 0; }
            char error_message[256];
            sprintf(error_message, "Lexical Error on Line # => %d. Unrecognized Lexeme: \"%s\"\n%s\n",
                    lexer->current_line, lexeme, dfa->state_to_error[previous_state]);
            report_error(LEXICAL_ERROR, error_message);
            return get_next_token(lexer);
        }
    }

    char unrecognized_character = lexeme[lexeme_length - 1];
    lexeme[lexeme_length - 1] = 0;
    // Character doesn't belong to alphabet, no need to retract
    if (lexeme_length >= 2) {
        // 2 errors: Current Lexical Error & Unrecognized Symbol Error
        char error_message[256];
        sprintf(error_message, "Lexical Error on Line # => %d. Unrecognized Lexeme: \"%s\"\n", lexer->current_line,
                lexeme);
        report_error(LEXICAL_ERROR, error_message);
    }
    char error_message[256];
    sprintf(error_message, "Lexical Error on Line # => %d Character does not belong to language alphabet: %c\n",
            lexer->current_line, unrecognized_character);
    report_error(LEXICAL_ERROR, error_message);

    free(lexeme);
    return get_next_token(lexer);
}

static Token *create_token_for(const char *lexeme, int line_number, int token_type) {
    Token *token = malloc_safe(sizeof(Token));
    token->lexeme = strdup(lexeme);
    token->line_number = line_number;
    token->token_type = token_type;
    return token;
}

void run_lexer_independently_on_and_print_to(Lexer *lexer, FILE *out) {
    int DOLLAR_TOKEN = DOLLAR;
    Token *t = get_next_token(lexer);
    while (t->token_type != DOLLAR_TOKEN) {
        print_token_to(lexer, t, out);
        t = get_next_token(lexer);
    }
}

static void print_token_to(Lexer *lexer, Token *t, FILE *out) {
    fprintf(out, "%-5s = %-5d %-5s = %-10s %-5s = %-5s\n",
            "Line #", t->line_number, "Lexeme", t->lexeme, "TokenType", SYMBOL_TO_STRING[t->token_type]);
}


static Token *get_dollar_token(const Lexer *lexer) {
    Token *t = malloc_safe(sizeof(Token));
    t->line_number = lexer->current_line;
    t->token_type = DOLLAR;
    t->lexeme = strdup(SYMBOL_TO_STRING[DOLLAR]);
    return t;
}

static void reload_buffer_for(Lexer *lexer) {
    if (lexer->buffer_number == 1) {
        lexer->buffer_number = 2;
        reload_buffer(lexer, lexer->buffer2);
    } else if (lexer->buffer_number == 2) {
        lexer->buffer_number = 1;
        reload_buffer(lexer, lexer->buffer1);
    }
    lexer->forward = 0;
}

static void reload_buffer(Lexer *lexer, char *buffer) {
    FILE *fp = lexer->file;
    // -1 so sentinel (DOLLAR) can be placed at the end
    size_t read = fread(buffer, sizeof(char), lexer->buffer_size - 1, fp);
    buffer[read] = DOLlAR;
}

void print_lexer(Lexer *lexer, FILE *fp) {
    fprintf(fp, "Lexer's DFA: \n");
    print_dfa_for(lexer, fp);
    fprintf(fp, "# Tokens = %d\n", N_TERMINALS);
    fprintf(fp, "Lexer's Token Types:\n");
    for (int i = 0; i < N_TERMINALS; ++i) {
        fprintf(fp, "%d: %s\n", i, SYMBOL_TO_STRING[i]);
    }
    fprintf(fp, "For source code from: %s\n", lexer->file_name);
}


void print_token(Lexer *lexer, Token *t) {
    printf("%-5s = %-5d %-5s = %-10s %-5s = %-5s\n",
           "Line #", t->line_number, "Lexeme", t->lexeme, "TokenType", SYMBOL_TO_STRING[t->token_type]);
}

void grant_symbol_table_to_lexer(Lexer *lexer, SymbolTable *symbol_table) {
    lexer->symbol_table = symbol_table;
}

void set_string_to_token_type_map_for_lexer(Lexer *lexer, HashMap *string_to_token_type_map) {
    lexer->string_to_token_type_map = string_to_token_type_map;
}
