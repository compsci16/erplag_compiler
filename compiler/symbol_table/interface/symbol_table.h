#ifndef COMPILER_SYMBOL_TABLE_H
#define COMPILER_SYMBOL_TABLE_H

#include "arraylist.h"
#include "grammar/grammar_populator/grammar_constants.h"
#include "ast_def.h"

#define IS_EMPTY_SLOT(symbol_table, index) ((symbol_table)->records[index] == NULL)
#define INITIAL_SYMBOL_TABLE_SIZE 2

typedef struct SymbolTable SymbolTable;
#define INIT_SYMBOL_TABLE_ENTRY_PTR(X) SymbolTableEntry *X = malloc_safe(sizeof(SymbolTableEntry)); \
                                       (X)->name = NULL; \
                                       (X)->line_number = 0; \
                                       (X)->was_assigned = false;                                   \
                                       (X)->is_static = false;                                \
                                       (X)->is_array = false;                                                             \
                                       (X)->width =0;                                                             \
                                       (X)->type_descriptor.form = TYPE_UNDEFINED;

#define TYPE_DESCRIPTOR_INIT(X) TypeDescriptor X = { .form = TYPE_UNDEFINED }

typedef struct SymbolTable {
    int prime_size_index;
    int current_offset;
    size_t capacity;
    int size;
    int total_data_size;
    struct SymbolTable *parent;
    struct SymbolTableEntry **records;
    int start_line;
    int end_line;
    // Add a children field to keep track of child symbol tables
    size_t num_children;
    size_t max_children;
    struct SymbolTable **children;
} SymbolTable;


typedef enum TypeForm {
    TYPE_INTEGER = INTEGER,
    TYPE_REAL = REAL,
    TYPE_BOOLEAN = BOOLEAN,
    TYPE_ARRAY,
    TYPE_FUNCTION,
    TYPE_KEYWORD,
    TYPE_UNDEFINED,
    TYPE_ERROR
} TypeForm;


typedef struct TypeDescriptor {
    TypeForm form;
    union {
        // form == TYPE_ARRAY
        struct ArrayType {
            TypeForm atomic_type;
            Range array_range;
            bool is_dynamic;
        } array_type;
        // form == TYPE_FUNCTION
        struct FunctionType {
            size_t num_returns;
            // there should only be atomic descriptors in return types
            struct TypeDescriptor *return_types;
            size_t num_parameters;
            struct TypeDescriptor *parameter_types;
            SymbolTable *symbol_table;
        } function_type;
    };
} TypeDescriptor;


typedef struct SymbolTableEntry {
    const char *name;
    int line_number;
    bool was_assigned;
    bool is_static;
    bool is_array;
    int width;
    struct TypeDescriptor type_descriptor;
    int offset;
} SymbolTableEntry;

SymbolTable *create_symbol_table();

SymbolTableEntry *search_in_cactus_stack(SymbolTable *current, char *name);

SymbolTableEntry *search_in_symbol_table(SymbolTable *symbol_table, char *name);

void print_symbol_table(SymbolTable *symbol_table, int depth);

//void print_symbol_table_as_required();

// caller's responsibility to free
char *type_descriptor_to_string(TypeDescriptor type_descriptor);

void print_symbol_table_as_required(SymbolTable *symbol_table);

void print_array_variable_entry(SymbolTable *global_symbol_table);

void print_total_memory_requirement(SymbolTable *global_symbol_table);

// puts the entry, overrides any previous entries with the same name
void insert_in_symbol_table(SymbolTable *symbol_table, SymbolTableEntry *entry);

// returns true if the two type descriptors are equivalent for static type checking
bool are_equivalent_type_descriptors(TypeDescriptor t1, TypeDescriptor t2);

void add_child_symbol_table(SymbolTable *parent, SymbolTable *child);

void set_max_scope_of_symbol_table(SymbolTable *parent);

void print_symbol_table_as_required(SymbolTable *global_symbol_table);

void destroy_symbol_table(SymbolTable *symbol_table);

#endif //COMPILER_SYMBOL_TABLE_H
