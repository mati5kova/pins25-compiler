//
// Created by Matevž Kovačič on 9. 7. 25.
//

#include "../include/error_utils.h"
#include "../include/lexer.h"
#include "../include/token_stream.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"

void printLexerError(const char* fileName, const char* errorMsg, const int line, const int column, const int position, const char* lexemStart, const int length) {
    fprintf(stdout, "%s [ln: %d, col: %d, pos: %d]: ", fileName, line, column, position);
    fprintf(stdout, "%serror:%s ", ANSI_COLOR_RED, ANSI_COLOR_RESET);
    fprintf(stdout, "%s\n", errorMsg);

    fprintf(stdout, "\t->\t%.*s\n", length, lexemStart);
}

void printVerboseInfo(char* additionalErrorMsg) {
    fprintf(stdout, "verbose: %s%s%s\n\n", ANSI_COLOR_RED, additionalErrorMsg, ANSI_COLOR_RESET);
}

void printHelp(char* title, char* msg) {
    fprintf(stdout, "--help:\n[%s]\n%s%s%s\n\n", title, ANSI_COLOR_YELLOW, msg, ANSI_COLOR_RESET);
}

void printSyntaxError(const char* fileName, const char* errorMsg, const Token* errToken) {
    fprintf(stdout, "%s [ln: %d, col: %d, pos: %d]: ", fileName, errToken->location->ln, errToken->location->col, errToken->location->pos);
    fprintf(stdout, "%serror:%s ", ANSI_COLOR_RED, ANSI_COLOR_RESET);
    fprintf(stdout, "%s\n", errorMsg);

    fprintf(stdout, "\t->\t");

    FILE* src = fopen(fileName, "r");
    if (src) {
        char* line = NULL; // buffer za vrstico ki jo izpisujemo
        size_t len = 0;
        int currentLine = 1;

        while (getline(&line, &len, src) != EOF) {
            if (currentLine == errToken->location->ln) {

                const size_t lineLength = strlen(line);
                if (lineLength && line[lineLength - 1] == '\n')
                {
                    line[lineLength - 1] = '\0'; // null terminiramo za izpis
                }

                // izpisemo vrstico z napacnim tokenom
                fprintf(stdout, "%s\n", line);

                // izpisemo ^ pod napacnim tokenom
                for (int i = 1; i < errToken->location->col; i++) {
                    fputc(line[i - 1] == '\t' ? '\t' : ' ', stdout);
                }
                fprintf(stdout, "\t\t  %s^%s\n", ANSI_COLOR_RED, ANSI_COLOR_RESET); // offset od fprintf(stdout, "\t->\t");

                free(line);
                break;
            }
            currentLine++;
        }
        fclose(src);
    }

}