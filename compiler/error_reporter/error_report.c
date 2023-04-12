#include "error_report.h"
#include "utility.h"
#include "compiler/semantic_analyzer/semantic_analyzer.h"

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

void report_error(ErrorType error_type, char *error_message) {
    switch (error_type) {
        case LEXICAL_ERROR: {
            printf(BLU);
            break;
        }
        case SYNTAX_ERROR: {
            printf(RED);
            break;
        }
        case SEMANTIC_ERROR: {
            GOT_SEMANTIC_ERROR_GLOBAL = true;
            printf(GRN);
            break;
        }
    }
    printf("%s", error_message);
    printf(RESET);
}
