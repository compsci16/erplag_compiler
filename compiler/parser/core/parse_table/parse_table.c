#include "parse_table.h"
#include "utility/utility.h"
#include <stdio.h>
#include "grammar/cfg.h"
#include "grammar/first_follow/first.h"
#include "utility/filehandler.h"
#include "grammar/grammar_populator/grammar_constants.h"

Production ***get_parse_table_for(const Grammar *g) {
    const int N_PRODUCTIONS = g->N_PRODUCTIONS;
    // SYMBOL By SYMBOL Matrix even though we will access only NT rows and T columns
    Production ***parse_table = malloc_safe(sizeof(Production **) * (N_NON_TERMINALS + N_TERMINALS));

    // Columns for every terminal:
    for (int i = NON_TERMINALS_BEGIN; i <= NON_TERMINALS_END; ++i) {
        parse_table[i] = malloc_safe(N_NON_TERMINALS + N_TERMINALS * sizeof(Production *));
    }

    // parse_table[i][j].rhs = 0 => error
    for (int i = NON_TERMINALS_BEGIN; i <= NON_TERMINALS_END; ++i) {
        for (int j = TERMINALS_BEGIN; j <= TERMINALS_END; ++j) {
            parse_table[i][j] = NULL;
        }
    }

    HashMap *follow_sets_map = g->follow_sets_map;

    for (int i = 0; i < N_PRODUCTIONS; ++i) {
        Production *current = g->productions[i];
        int which_row = current->lhs;
        if (!is_null_production(current)) {
            HashSet *first_set_rhs = first_set_of_symbols(g, current->rhs, current->rhs_size);
            HashSetIterator *first_set_iterator = create_hashset_iterator(first_set_rhs);
            const int *first_set_element;
            while ((first_set_element = get_next_hashset(first_set_iterator)) != NULL) {
                if (*first_set_element != EPSILON) {
                    int which_column = *first_set_element;
                    if (parse_table[which_row][which_column] != NULL) {
                        fprintf(stderr,
                                "2 productions in cell [%d][%d]: Symbols: [NT:%s] [T:%s]\n", which_row, which_column,
                                SYMBOL_TO_STRING[which_row],
                                SYMBOL_TO_STRING[which_column]);
                        fprintf(stderr, "Grammar is not LL1 compatible.");
                        destroy_hashset_iterator(first_set_iterator);
                        destroy_parse_table(g, parse_table);
                        return NULL;
                    }

                    size_t prod_size = sizeof(Production) + sizeof(int) * current->rhs_size;
                    parse_table[which_row][which_column] = malloc_safe(prod_size);
                    parse_table[which_row][which_column]->lhs = current->lhs;
                    parse_table[which_row][which_column]->rhs_size = current->rhs_size;
                    for (int j = 0; j < current->rhs_size; ++j) {
                        parse_table[which_row][which_column]->rhs[j] = current->rhs[j];
                    }
                }
            }
            destroy_hashset(first_set_rhs);
            destroy_hashset_iterator(first_set_iterator);
        } else {
            HashSet **follow_set_of_lhs = get_from_hashmap(follow_sets_map, &current->lhs);
            HashSetIterator *follow_set_iterator = create_hashset_iterator(*follow_set_of_lhs);
            const int *follow_set_element;
            while ((follow_set_element = get_next_hashset(follow_set_iterator)) != NULL) {
                int which_column = *follow_set_element;

                if (parse_table[which_row][which_column] != NULL) {
                    fprintf(stderr, "Grammar is not LL1 compatible.");
                    destroy_hashset_iterator(follow_set_iterator);
                    destroy_parse_table(g, parse_table);
                    return NULL;
                }

                size_t prod_size = sizeof(Production) + sizeof(int) * current->rhs_size;
                parse_table[which_row][which_column] = malloc_safe(prod_size);
                parse_table[which_row][which_column]->lhs = current->lhs;
                parse_table[which_row][which_column]->rhs_size = current->rhs_size;
                for (int j = 0; j < current->rhs_size; ++j) {
                    parse_table[which_row][which_column]->rhs[j] = current->rhs[j];
                }
            }
            destroy_hashset_iterator(follow_set_iterator);
        }
    }
    return parse_table;
}

void print_parse_table(Grammar *g, Production ***table, char *path) {
    FILE *fp = fopen(path, "w");
    if (!fp) {
        report_file_error(path);
        return;
    }
    char *format = "%s,";
    // Print the table header
    fprintf(fp, format, "Non Terminal");
    for (int i = TERMINALS_BEGIN; i <= TERMINALS_END; ++i) {
        fprintf(fp, format, SYMBOL_TO_STRING[i]);
    }
    fprintf(fp, "\n");
    for (int i = NON_TERMINALS_BEGIN; i <= NON_TERMINALS_END; ++i) {
        fprintf(fp, format, SYMBOL_TO_STRING[i]);
        for (int j = TERMINALS_BEGIN; j <= TERMINALS_END; ++j) {
            if (table[i][j] == NULL) {
                fprintf(fp, format, "Error ");
            } else {
                char *production_string = get_production_string(g, table[i][j]);
                fprintf(fp, format, production_string);
                free(production_string);
            }
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
}

void destroy_parse_table(const Grammar *g, Production ***parse_table) {
    for (int j = 0; j < N_NON_TERMINALS; ++j) {
        for (int k = 0; k < N_TERMINALS; ++k) {
            free(parse_table[j][k]);
        }
        free(parse_table[j]);
    }
}