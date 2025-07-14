//
// Created by Matevž Kovačič on 9. 7. 25.
//

#include "../include/error_utils.h"
#include <stdio.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"

void printError(const char* fileName, const char* errorType, const int line, const int column, const int position, const char* lexemStart, const int length) {
    fprintf(stdout, "%s [ln: %d, col: %d, pos: %d]: ", fileName, line, column, position);
    fprintf(stdout, "%serror:%s ", ANSI_COLOR_RED, ANSI_COLOR_RESET);
    fprintf(stdout, "%s\n", errorType);

    fprintf(stdout, "\t->\t%.*s\n", length, lexemStart);
}

void printVerboseInfo(char* additionalErrorMsg) {
    fprintf(stdout, "--verbose:\n%s%s%s\n\n", ANSI_COLOR_RED, additionalErrorMsg, ANSI_COLOR_RESET);
}

void printHelp(char* title, char* msg) {
    fprintf(stdout, "--help:\n[%s]\n%s%s%s\n\n", title, ANSI_COLOR_YELLOW, msg, ANSI_COLOR_RESET);
}
