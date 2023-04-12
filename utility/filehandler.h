#ifndef COMPILER_FILEHANDLER_H
#define COMPILER_FILEHANDLER_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char **split_by(char *string, const char *by, int *num_split, int max_strings_in_result);

void report_file_error(const char *path);

#endif //COMPILER_FILEHANDLER_H
