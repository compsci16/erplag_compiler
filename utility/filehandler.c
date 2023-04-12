#include "filehandler.h"

char **split_by(char *string, const char *by, int *num_split, int max_strings_in_result) {
    char **result = malloc(max_strings_in_result * sizeof(char *));
    char *token = strtok(string, by);
    int count = 0;
    while (token != NULL) {
        result[count] = strdup(token);
        // printf("%s\n", token);
        token = strtok(NULL, by);
        ++count;
    }
    *num_split = count;
    return result;
}

void report_file_error(const char *path) {
    fprintf(stderr, "Unable to read %s file\n", path);
}


