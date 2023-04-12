#include <stdbool.h>
#include "filehandler.h"

void remove_comments_from_source_code(FILE *source, FILE *destination) {
    char buffer[1024];
    char *start, *end;
    bool isInComment = false;

    while (fgets(buffer, sizeof(buffer), source)) {
        int i = 0;
        while (buffer[i] != '\0') {
            if (!isInComment && buffer[i] == '*' && buffer[i + 1] == '*') {
                start = &buffer[i];
                isInComment = 1;
                i += 2;
            } else if (isInComment && buffer[i] == '*' && buffer[i + 1] == '*') {
                end = &buffer[i + 1];
                memmove(start, end + 1, strlen(end));
                isInComment = 0;
                i = start - buffer;
            } else {
                i++;
            }
        }
        if (!isInComment) {
            fputs(buffer, destination);
        }
    }
}