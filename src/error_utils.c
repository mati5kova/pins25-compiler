//
// Created by Matevž Kovačič on 9. 7. 25.
//

#include "../include/error_utils.h"
#include <stdio.h>

#define COLOR_RED     "\x1b[31m"
#define COLOR_RESET   "\x1b[0m"

void printError(const char* fileName, const char* errorType, const int line, const int column, const int position, const char* lexemStart, int length) {

    fprintf(stderr, "%s [ln: %d, col: %d, pos: %d]: ", fileName, line, column, position);
    fprintf(stderr, "%serror%s: %s\n", COLOR_RED, COLOR_RESET, errorType);

    fprintf(stderr, "\t->\t%.*s\n", length, lexemStart);
}
