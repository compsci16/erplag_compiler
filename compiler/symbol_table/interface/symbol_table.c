#include "symbol_table.h"
#include "utility.h"
#include "compiler/error_reporter/error_report.h"

#define MAX_SYMBOL_TABLE_CHILDREN_INITIAL 4
static const size_t PRIMES[] = {2, 5, 11, 23, 47, 97, 197, 397, 797, 1597, 3203, 6421, 12853, 25717, 51437, 102877,
                                205759};


#define LOAD_FACTOR(symbol_table) ((double)(symbol_table)->size / (double)(symbol_table)->capacity)
#define PROBING_SEQUENCE(x) ((x)*(x))

static char *function_descriptor_to_string(TypeDescriptor array_descriptor);

static void print_entry(SymbolTableEntry *entry, int depth);

static void print_symbol_table_local(SymbolTable *symbol_table);

static void print_symbol_table_as_required_helper(SymbolTable *symbol_table, int nesting_level, int start_line,
                                                  int end_line, char *module_name);


static void print_array_variable_entry_helper(SymbolTable *symbol_table, int nesting_level, int start_line,
                                              int end_line, char *module_name);


static char *array_descriptor_to_string(TypeDescriptor array_descriptor);

static char *atomic_type_to_string(TypeForm type);

static void grow_symbol_table(SymbolTable *symbol_table);

static size_t hash(SymbolTable *symbol_table, const char *str);

SymbolTable *create_symbol_table() {
    SymbolTable *symbol_table = malloc_safe(sizeof(SymbolTable));
    symbol_table->parent = NULL;
    symbol_table->records = malloc_safe(sizeof(SymbolTableEntry) * INITIAL_SYMBOL_TABLE_SIZE);
    symbol_table->size = 0;
    symbol_table->capacity = INITIAL_SYMBOL_TABLE_SIZE;
    symbol_table->prime_size_index = 0;
    symbol_table->num_children = 0;
    symbol_table->max_children = MAX_SYMBOL_TABLE_CHILDREN_INITIAL;
    symbol_table->current_offset = 0;
    symbol_table->children = malloc_safe(sizeof(SymbolTable *) * MAX_SYMBOL_TABLE_CHILDREN_INITIAL);
    symbol_table->total_data_size = 0;
    for (size_t i = 0; i < symbol_table->capacity; ++i) {
        symbol_table->records[i] = NULL;
    }
    return symbol_table;
}

void insert_in_symbol_table(SymbolTable *symbol_table, SymbolTableEntry *entry) {
    const char *name = entry->name;
    size_t x = 1;
    size_t key_hash = hash(symbol_table, name);
    size_t index = key_hash;
    while (!IS_EMPTY_SLOT(symbol_table, index)) {
        if (strcmp(symbol_table->records[index]->name, name) == 0) {
            symbol_table->records[index] = entry;
            return;
        }
        index = (key_hash + PROBING_SEQUENCE(x)) % symbol_table->capacity;
        x++;
    }
    symbol_table->records[index] = entry;
    symbol_table->size++;
    if (LOAD_FACTOR(symbol_table) >= 0.5) {
        grow_symbol_table(symbol_table);
    }
}

SymbolTableEntry *search_in_symbol_table(SymbolTable *symbol_table, char *name) {
    size_t x = 1;
    size_t key_hash = hash(symbol_table, name);
    size_t index = key_hash;
    while (!IS_EMPTY_SLOT(symbol_table, index)) {
        if (strcmp(symbol_table->records[index]->name, name) == 0) {
            return symbol_table->records[index];
        }
        index = (key_hash + PROBING_SEQUENCE(x)) % symbol_table->capacity;
        x++;
    }
    return NULL;
}

SymbolTableEntry *search_in_cactus_stack(SymbolTable *current, char *name) {
    if (current == NULL) return NULL;
    SymbolTableEntry *symbol_table_entry = search_in_symbol_table(current, name);
    if (symbol_table_entry != NULL) return symbol_table_entry;
    else return search_in_cactus_stack(current->parent, name);
}

void print_total_memory_requirement(SymbolTable *global_symbol_table) {
    for (int i = 0; i < global_symbol_table->capacity; ++i) {
        SymbolTableEntry *record = global_symbol_table->records[i];
        if (record == NULL) continue;
        char *mod_name = record->name;
        SymbolTable *function_symbol_table = record->type_descriptor.function_type.symbol_table;
        int total_size = function_symbol_table->total_data_size;
        printf("%-10s%-10d\n", mod_name, total_size);
    }
}

static void print_symbol_table_local(SymbolTable *symbol_table) {
    puts("Symbol Table");
    SymbolTableEntry **records = symbol_table->records;
    for (size_t i = 0; i < symbol_table->capacity; ++i) {
        if (records[i] != NULL) {
            printf("%zu. %s\n", i, records[i]->name);
        } else {
            printf("%zu. NULL\n", i);
        }
    }
}

// For official test cases
void print_symbol_table_as_required(SymbolTable *global_symbol_table) {
    printf("%-15s%-20s%-20s%-28s%-10s%-20s%-20s%-10s%-10s%-15s\n", "variable name", "scope[module_name]",
           "scope[line #s]",
           "type", "is_array",
           "static/dynamic", "array range", "width", "offset", "nesting level");

    for (int i = 0; i < global_symbol_table->capacity; ++i) {
        SymbolTableEntry *record = global_symbol_table->records[i];
        if (record == NULL) continue;
        char *mod_name = record->name;
        int s = record->type_descriptor.function_type.symbol_table->start_line;
        int e = record->type_descriptor.function_type.symbol_table->end_line;
        print_symbol_table_as_required_helper(record->type_descriptor.function_type.symbol_table,
                                              0, s, e, mod_name);
    }
}


static void print_symbol_table_as_required_helper(SymbolTable *symbol_table, int nesting_level, int start_line,
                                                  int end_line, char *module_name) {
    SymbolTableEntry **records = symbol_table->records;
    for (int i = 0; i < symbol_table->capacity; ++i) {
        if (records[i] != NULL) {
            char *is_array = symbol_table->records[i]->is_array ? "yes" : "no";
            char *static_or_dyn = symbol_table->records[i]->is_array ?
                                  (symbol_table->records[i]->is_static ? "static" : "dynamic") : "**";
            int width = symbol_table->records[i]->width;
            switch (records[i]->type_descriptor.form) {
                case TYPE_INTEGER: {
                    printf("%-15s%-20s%-3d%-3s%-14d%-28s%-10s%-20s%-20s%-10d%-10d%-10d\n", records[i]->name,
                           module_name,
                           start_line, "-", end_line, "integer", is_array,
                           static_or_dyn, "**", width, records[i]->offset, nesting_level);
                    break;
                }
                case TYPE_REAL: {
                    printf("%-15s%-20s%-3d%-3s%-14d%-28s%-10s%-20s%-20s%-10d%-10d%-10d\n", records[i]->name,
                           module_name,
                           start_line, "-", end_line, "real", is_array,
                           static_or_dyn, "**", width, records[i]->offset, nesting_level);
                    break;
                }
                case TYPE_BOOLEAN:
                    printf("%-15s%-20s%-3d%-3s%-14d%-28s%-10s%-20s%-20s%-10d%-10d%-10d\n", records[i]->name,
                           module_name,
                           start_line, "-", end_line, "boolean", is_array,
                           static_or_dyn, "**", width, records[i]->offset, nesting_level);
                    break;
                case TYPE_ARRAY: {
                    if (records[i]->is_static) {
                        int low = atoi(records[i]->type_descriptor.array_type.array_range.begin->lexeme);
                        int high = atoi(records[i]->type_descriptor.array_type.array_range.end->lexeme);
                        if (records[i]->type_descriptor.array_type.array_range.begin_sign->token_type == MINUS) {
                            low = (-1) * low;
                        }
                        if (records[i]->type_descriptor.array_type.array_range.end_sign->token_type == MINUS) {
                            high = (-1) * high;
                        }
                        printf("%-15s%-20s%-3d%-3s%-14d%-28s%-10s%-20s%-3d%-3s%-14d%-10d%-10d%-10d\n", records[i]->name,
                               module_name,
                               start_line, "-", end_line, type_descriptor_to_string(records[i]->type_descriptor),
                               is_array,
                               static_or_dyn, low, "-", high, width, records[i]->offset, nesting_level);
                    } else {
                        char *low = records[i]->type_descriptor.array_type.array_range.begin->lexeme;
                        char *high = records[i]->type_descriptor.array_type.array_range.end->lexeme;
                        printf("%-15s%-20s%-3d%-3s%-14d%-28s%-10s%-20s%-3s%-3s%-14s%-10d%-10d%-10d\n", records[i]->name,
                               module_name,
                               start_line, "-", end_line, type_descriptor_to_string(records[i]->type_descriptor),
                               is_array,
                               static_or_dyn, low, "-", high, width, records[i]->offset, nesting_level);
                    }
                    break;
                }
                case TYPE_FUNCTION:
                    break;
                case TYPE_KEYWORD:
                case TYPE_UNDEFINED:
                case TYPE_ERROR:
                    break;
            }
        }
    }

    for (int i = 0; i < symbol_table->num_children; ++i) {
        print_symbol_table_as_required_helper(symbol_table->children[i], nesting_level + 1,
                                              symbol_table->children[i]->start_line,
                                              symbol_table->children[i]->end_line, module_name);
    }
}

void print_array_variable_entry(SymbolTable *global_symbol_table) {
    printf("%-15s%-20s%-20s%-28s%-20s%-20s%-10s\n", "variable name", "scope[module_name]",
           "scope[line #s]",
           "type",
           "static/dynamic", "array range", "width");
    for (int i = 0; i < global_symbol_table->capacity; ++i) {
        SymbolTableEntry *record = global_symbol_table->records[i];
        if (record == NULL) continue;
        char *mod_name = record->name;
        int s = record->type_descriptor.function_type.symbol_table->start_line;
        int e = record->type_descriptor.function_type.symbol_table->end_line;
        print_symbol_table_as_required_helper(record->type_descriptor.function_type.symbol_table,
                                              0, s, e, mod_name);
    }

}

static void print_array_variable_entry_helper(SymbolTable *symbol_table, int nesting_level, int start_line,
                                              int end_line, char *module_name) {
    SymbolTableEntry **records = symbol_table->records;
    for (int i = 0; i < symbol_table->capacity; ++i) {
        if (records[i] != NULL && records[i]->type_descriptor.form != TYPE_ARRAY) {
            char *is_array = symbol_table->records[i]->is_array ? "yes" : "no";
            char *static_or_dyn = symbol_table->records[i]->is_array ?
                                  (symbol_table->records[i]->is_static ? "static" : "dynamic") : "**";
            int width = symbol_table->records[i]->width;
            if (records[i]->is_static) {
                int low = atoi(records[i]->type_descriptor.array_type.array_range.begin->lexeme);
                int high = atoi(records[i]->type_descriptor.array_type.array_range.end->lexeme);
                if (records[i]->type_descriptor.array_type.array_range.begin_sign->token_type == MINUS) {
                    low = (-1) * low;
                }
                if (records[i]->type_descriptor.array_type.array_range.end_sign->token_type == MINUS) {
                    high = (-1) * high;
                }
                printf("%-15s%-20s%-3d%-3s%-14d%-28s%-20s%-3d%-3s%-14d%-10d%\n", records[i]->name,
                       module_name,
                       start_line, "-", end_line, type_descriptor_to_string(records[i]->type_descriptor),
                       static_or_dyn, low, "-", high, width);
            } else {
                char *low = records[i]->type_descriptor.array_type.array_range.begin->lexeme;
                char *high = records[i]->type_descriptor.array_type.array_range.end->lexeme;
                printf("%-15s%-20s%-3d%-3s%-14d%-28s%-20s%-3s%-3s%-14s%-10d\n", records[i]->name,
                       module_name,
                       start_line, "-", end_line, type_descriptor_to_string(records[i]->type_descriptor),
                       static_or_dyn, low, "-", high, width);
            }
        }
    }

    for (int i = 0; i < symbol_table->num_children; ++i) {
        print_symbol_table_as_required_helper(symbol_table->children[i], nesting_level + 1,
                                              symbol_table->children[i]->start_line,
                                              symbol_table->children[i]->end_line, module_name);
    }
}


void print_symbol_table(SymbolTable *symbol_table, int depth) {
    for (int i = 0; i < depth; ++i) {
        printf("-");
    }
    printf("Symbol Table , total data size = %d, current offset = %d, size = %d"
           "\n", symbol_table->total_data_size, symbol_table->current_offset,
           symbol_table->size);
    for (int i = 0; i < depth; ++i) {
        printf(" ");
    }
    printf("%-10s%-10s%-10s%-15s%-10s%-10s%-10s\n", "name", "line #", "type", "was_assigned", "info", "offset",
           "width");
    SymbolTableEntry **records = symbol_table->records;
    for (size_t i = 0; i < symbol_table->capacity; i++) {
        if (records[i] != NULL) {
            print_entry(records[i], depth);
        }
    }

    // Recursively call print_symbol_table on any child symbol tables
    for (size_t i = 0; i < symbol_table->num_children; i++) {
        SymbolTable *child = symbol_table->children[i];
        print_symbol_table(child, depth + 60);
    }
}

static void print_entry(SymbolTableEntry *entry, int depth) {
    for (int i = 0; i < depth; ++i) {
        printf("-");
    }
    int width = entry->width;
    switch (entry->type_descriptor.form) {
        case TYPE_INTEGER:
        case TYPE_REAL:
        case TYPE_BOOLEAN: {
            printf("%-10s%-10d%-10s%-15d%-10s%-10d%-10d\n",
                   entry->name, entry->line_number, SYMBOL_TO_STRING[entry->type_descriptor.form],
                   entry->was_assigned, "X", entry->offset, width);
            break;
        }
        case TYPE_ARRAY: {
            const char *str = SYMBOL_TO_STRING[entry->type_descriptor.array_type.atomic_type];
            char *r0 = ":";
            char *r1 = entry->type_descriptor.array_type.array_range.begin_sign->lexeme;
            char *r2 = entry->type_descriptor.array_type.array_range.begin->lexeme;
            char *r3 = entry->type_descriptor.array_type.array_range.end_sign->lexeme;
            char *r4 = entry->type_descriptor.array_type.array_range.end->lexeme;

            // Concatenate the strings using sprintf
            char result[1024];
            sprintf(result, "%s%s%s%s%s%s%s", str, r0, r1, r2, " to ", r3, r4);
            printf("%-10s%-10d%-10s%-15d%-20s%-10d%-10d\n", entry->name, entry->line_number, "ArrayType",
                   entry->was_assigned,
                   result, entry->offset, width);
            break;
        }
        case TYPE_FUNCTION: {
            printf("%-10s%-10d%-15s%-10d", entry->name, entry->line_number, "function", entry->was_assigned);

            // Print return types
            printf("%-10s", "returns:");
            for (size_t i = 0; i < entry->type_descriptor.function_type.num_returns; ++i) {
                printf("%s ", SYMBOL_TO_STRING[entry->type_descriptor.function_type.return_types[i].form]);
            }
            struct FunctionType function_type = entry->type_descriptor.function_type;
            // Print parameter types
            printf("%s%-10s", "", ";parameters:");
            for (size_t i = 0; i < entry->type_descriptor.function_type.num_parameters; ++i) {
                if (function_type.parameter_types[i].form != TYPE_ARRAY) {
                    printf("%s ", SYMBOL_TO_STRING[function_type.parameter_types[i].form]);
                } else {
                    struct ArrayType array_type = entry->type_descriptor.function_type.parameter_types[i].array_type;
                    printf("%s:[%s%s%s%s] ",
                           SYMBOL_TO_STRING[array_type.atomic_type],
                           array_type.array_range.begin_sign->lexeme,
                           array_type.array_range.begin->lexeme,
                           array_type.array_range.end_sign->lexeme,
                           array_type.array_range.end->lexeme);
                }
            }
            printf("\n");
            break;
        }
        case TYPE_KEYWORD:
        case TYPE_UNDEFINED:
            break;
        case TYPE_ERROR:
            break;
    }
}

void destroy_symbol_table(SymbolTable *symbol_table) {
    for (int i = 0; i < symbol_table->capacity; ++i) {
        free(symbol_table->records[i]);
    }
    for (int i = 0; i < symbol_table->num_children; ++i) {
        destroy_symbol_table(symbol_table->children[i]);
    }
    free(symbol_table);
}

static void grow_symbol_table(SymbolTable *symbol_table) {
    symbol_table->size = 0;
    size_t old_capacity = symbol_table->capacity;
    symbol_table->capacity = PRIMES[++symbol_table->prime_size_index];
    SymbolTableEntry **old_records = symbol_table->records;
    SymbolTableEntry **new_records = malloc_safe(sizeof(SymbolTableEntry) * symbol_table->capacity);
    symbol_table->records = new_records;
    for (size_t i = 0; i < symbol_table->capacity; ++i) {
        new_records[i] = NULL;
    }
    for (size_t i = 0; i < old_capacity; ++i) {
        if (old_records[i] != NULL) {
            insert_in_symbol_table(symbol_table, old_records[i]);
        }
    }
    free(old_records);
}

static size_t hash(SymbolTable *symbol_table, const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++) != 0)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash % symbol_table->capacity;
}

// Function to add a child symbol table to a parent symbol table
void add_child_symbol_table(SymbolTable *parent, SymbolTable *child) {
    // Check if the children array needs to be resized
    if (parent->num_children == parent->max_children) {
        parent->max_children *= 2;
        parent->children = realloc(parent->children, sizeof(SymbolTable *) * parent->max_children);
    }
    // Add the child symbol table to the children array
    parent->children[parent->num_children++] = child;
}

void set_max_scope_of_symbol_table(SymbolTable *parent) {

}


static bool are_equivalent_array_types(struct ArrayType t1, struct ArrayType t2);

static bool are_equivalent_function_types(struct FunctionType t1, struct FunctionType t2);

bool are_equivalent_type_descriptors(TypeDescriptor t1, TypeDescriptor t2) {
    if (t1.form != t2.form) return false;
    switch (t1.form) {
        case TYPE_INTEGER:
        case TYPE_REAL:
        case TYPE_BOOLEAN:
            return true;
        case TYPE_ARRAY: {
            struct ArrayType t1_arr_type = t1.array_type;
            struct ArrayType t2_arr_type = t2.array_type;
            return are_equivalent_array_types(t1_arr_type, t2_arr_type);
        }
        case TYPE_FUNCTION: {
            struct FunctionType function_type1 = t1.function_type;
            struct FunctionType function_type2 = t2.function_type;
            return are_equivalent_function_types(function_type1, function_type2);
        }
        case TYPE_KEYWORD:
            break;
        case TYPE_UNDEFINED:
            break;
        case TYPE_ERROR:
            break;
    }
}

static bool are_equivalent_array_types(struct ArrayType t1, struct ArrayType t2) {
    Range r1 = t1.array_range;
    Range r2 = t2.array_range;
    // why NUM? For static type checking only!
    if (r1.begin->token_type != NUM || r1.end->token_type != NUM || r2.begin->token_type != NUM
        || r2.end->token_type != NUM)
        return true;

    if (t1.atomic_type != t2.atomic_type) return false;

    int left1 = atoi(r1.begin->lexeme);
    int right1 = atoi(r1.end->lexeme);
    if (r1.begin_sign->token_type == MINUS) {
        left1 = (-1) * left1;
    }
    if (r1.end_sign->token_type == MINUS) {
        right1 = (-1) * right1;
    }

    int left2 = atoi(r2.begin->lexeme);
    int right2 = atoi(r2.end->lexeme);
    if (r2.begin_sign->token_type == MINUS) {
        left2 = (-1) * left2;
    }
    if (r2.end_sign->token_type == MINUS) {
        right2 = (-1) * right2;
    }

    if (right1 - left1 != right2 - left2) return false;
    return true;
}

static bool are_equivalent_function_types(struct FunctionType t1, struct FunctionType t2) {
    if (t1.num_parameters != t2.num_parameters || t1.num_returns != t2.num_returns) return false;
    for (int i = 0; i < t1.num_parameters; ++i) {
        TypeDescriptor td1 = t1.parameter_types[i];
        TypeDescriptor td2 = t2.parameter_types[i];
        if (!are_equivalent_type_descriptors(td1, td2)) return false;
    }

    for (int i = 0; i < t1.num_returns; ++i) {
        TypeForm td1 = t1.return_types[i].form;
        TypeForm td2 = t2.return_types[i].form;
        if (td1 != td2) return false;
    }
    return true;
}


char *type_descriptor_to_string(TypeDescriptor type_descriptor) {
    switch (type_descriptor.form) {
        case TYPE_INTEGER:
        case TYPE_REAL:
        case TYPE_BOOLEAN:
            return atomic_type_to_string(type_descriptor.form);
        case TYPE_ARRAY:
            return array_descriptor_to_string(type_descriptor);
        case TYPE_FUNCTION:
            return function_descriptor_to_string(type_descriptor);
        case TYPE_KEYWORD:
            return "Keyword";
        case TYPE_UNDEFINED: {
            char *undefined = malloc_safe(sizeof(char) * 12);
            snprintf(undefined, sizeof(char) * 12, "Undefined");
        }
        default:
            return NULL;
    }
}

static char *atomic_type_to_string(TypeForm type) {
    const size_t MAX_SIZE_DESC = 16;
    char *string = malloc_safe(sizeof(char) * MAX_SIZE_DESC);
    switch (type) {
        case TYPE_INTEGER: {
            snprintf(string, MAX_SIZE_DESC, "integer");
            break;
        }
        case TYPE_REAL: {
            snprintf(string, MAX_SIZE_DESC, "real");
            break;
        }
        case TYPE_BOOLEAN: {
            snprintf(string, MAX_SIZE_DESC, "boolean");
            break;
        }
        default:
            return NULL;
    }
    return string;
}

static char *array_descriptor_to_string(TypeDescriptor array_descriptor) {
    const size_t MAX_DESC_SIZE = 128;
    char *string = calloc_safe(MAX_DESC_SIZE, sizeof(char));
    struct ArrayType array_type = array_descriptor.array_type;
    snprintf(string, MAX_DESC_SIZE, "%s%s,%s%s%s%s%s%s", "Array<",
             atomic_type_to_string(array_type.atomic_type),
             array_type.array_range.begin_sign->lexeme,
             array_type.array_range.begin->lexeme,
             ":",
             array_type.array_range.end_sign->lexeme,
             array_type.array_range.end->lexeme,
             ">");
    return string;
}

static char *function_descriptor_to_string(TypeDescriptor function_descriptor) {
    const size_t MAX_DESC_SIZE = 1024;
    char *string = calloc_safe(MAX_DESC_SIZE, sizeof(char));
    struct FunctionType function_type = function_descriptor.function_type;

    // Append name and line number
    snprintf(string, MAX_DESC_SIZE, "%-10s", "FunctionType<");

    // Append return types
    strncat(string, "Returns:(", MAX_DESC_SIZE - strlen(string) - 1);
    for (size_t i = 0; i < function_type.num_returns; ++i) {
        strncat(string, SYMBOL_TO_STRING[function_type.return_types[i].form], MAX_DESC_SIZE - strlen(string) - 1);
        strncat(string, ",", MAX_DESC_SIZE - strlen(string) - 1);
    }
    strncat(string, ")", 1);
    // Append parameter types
    strncat(string, " parameters:(", MAX_DESC_SIZE - strlen(string) - 1);
    for (size_t i = 0; i < function_type.num_parameters; ++i) {
        if (function_type.parameter_types[i].form != TYPE_ARRAY) {
            strncat(string, SYMBOL_TO_STRING[function_type.parameter_types[i].form],
                    MAX_DESC_SIZE - strlen(string) - 1);
            strncat(string, " ", MAX_DESC_SIZE - strlen(string) - 1);
        } else {
            struct ArrayType array_type = function_type.parameter_types[i].array_type;
            char array_string[MAX_DESC_SIZE];
            snprintf(array_string, MAX_DESC_SIZE, "%s:[%s%s%s%s],",
                     SYMBOL_TO_STRING[array_type.atomic_type],
                     array_type.array_range.begin_sign->lexeme,
                     array_type.array_range.begin->lexeme,
                     array_type.array_range.end_sign->lexeme,
                     array_type.array_range.end->lexeme);
            strncat(string, array_string, MAX_DESC_SIZE - strlen(string) - 1);
        }
        strncat(string, ")", 1);
    }

    return string;
}