//
// Created by Matevz on 03/07/2025.
//
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "../include/lexer.h"

#include <ctype.h>

#define INITIAL_TOKEN_COUNT 100
#define TOKEN_COUNT_INCREMENT 25

#define INITIAL_BUFFER_SIZE 500
#define BUFFER_SIZE_INCREMENT 250

uint32_t numberOfInputChars = 0;

uint32_t tokenCount = 0;

uint32_t skipUntilLineEnd(const char* src) {
    uint32_t counter = 0;
    while (src[counter] != '\n' && src[counter] != '\0') {
        counter++;
    }

    return counter;
}

uint32_t retrieveTokenCount() {
    return tokenCount;
}

char* stringifyInputFile(FILE* inputFile) {

    rewind(inputFile);
    numberOfInputChars = 0;

    uint32_t currentMaxBufferSIze = INITIAL_BUFFER_SIZE;
    char* buffer = malloc(INITIAL_BUFFER_SIZE * sizeof(char));
    if (!buffer) {
        printf("Out of memory [lexer.c stringifyInputFile]\n");
        exit(EXIT_FAILURE);
    }

    uint32_t c;
    while ((c = fgetc(inputFile)) != EOF) {
        if (c == '\r') {
            continue;
        }

        if (numberOfInputChars + 1 >= currentMaxBufferSIze) {
            char* temp = realloc(buffer, currentMaxBufferSIze + BUFFER_SIZE_INCREMENT);
            if (!temp) {
                free(buffer);
                printf("Failed to reallocate buffer\n");
                exit(EXIT_FAILURE);
            }
            buffer = temp;

            currentMaxBufferSIze += BUFFER_SIZE_INCREMENT;
        }

        buffer[numberOfInputChars] = (char) c;

        numberOfInputChars++;
    }

    buffer[numberOfInputChars] = '\0';

    return buffer;
}

Token** tokenize(FILE* inputFile) {
    uint32_t ln = 1;
    uint32_t col = 1;
    uint32_t pos = 0;

    printf("Hello from lexer.c\n");

    Token** tokens = malloc(INITIAL_TOKEN_COUNT * sizeof(Token*));
    if (!tokens) {
        printf("Out of memory [lexer.c tokenize]\n");
        exit(EXIT_FAILURE);
    }


    char* source = stringifyInputFile(inputFile);

    while (source[pos] != '\0' && pos < numberOfInputChars) {
        const char c = source[pos];

        if (c == ' ') {
            pos++;
            continue;
        }

        // handlamo novo vrstico
        if (c == '\n') {
            ln++;
            col = 1;
            pos++;
            printf("\n");
            continue;
        }

        // preverimo za komentar
        if (c == '/' && source[pos + 1] == '/') {
            pos += skipUntilLineEnd(&(source[pos]));
            continue;
        }

        // negativna števila
        if (c == '-') {

        }

        // cela števila
        if (isdigit(c)) {

        }

        // znak
        if (c == '\'') {

        }

        // keywords in identifiers
        if (isalpha(c)) {


        }

        if (c == '&') {
            if (source[pos + 1] == '&') {
                printf("[TOKEN_SYMBOL_LOGICAL_AND ");
                pos++;
            } else {
                printf("[TOKEN_ERROR ");
            }
        } else if (c == '|') {
            if (source[pos + 1] == '|') {
                printf("[TOKEN_SYMBOL_LOGICAL_OR ");
                pos++;
            } else {
                printf("[TOKEN_ERROR ");
            }
        } else if (c == '!') {
            if (source[pos + 1] == '=') {
                printf("[TOKEN_SYMBOL_LOGICAL_NOT_EQUALS ");
                pos++;
            } else {
                printf("[TOKEN_SYMBOL_LOGICAL_NOT ");
            }
        } else if (c == '=') {
            if (source[pos + 1] == '=') {
                printf("[TOKEN_SYMBOL_LOGICAL_EQUALS ");
                pos++;
            } else {
                printf("[TOKEN_SYMBOL_ASSIGN ");
            }
        } else if (c == '>') {
            if (source[pos + 1] == '=') {
                printf("[TOKEN_SYMBOL_LOGICAL_GREATER_OR_EQUALS ");
                pos++;
            } else {
                printf("[TOKEN_SYMBOL_LOGICAL_GREATER ");
            }
        } else if (c == '<') {
            if (source[pos + 1] == '=') {
                printf("[TOKEN_SYMBOL_LOGICAL_LESS_OR_EQUALS ");
                pos++;
            } else {
                printf("[TOKEN_SYMBOL_LOGICAL_LESS ");
            }
        } else {
            // za posamezne znake oz. operatorje...
            switch (c) {
                case ',':
                    printf("[TOKEN_SYMBOL_COMMA ");
                break;
                case '+':
                    printf("[TOKEN_SYMBOL_ARITHMETIC_PLUS ");
                break;
                case '-':
                    printf("[TOKEN_SYMBOL_ARITHMETIC_MINUS ");
                break;
                case '*':
                    printf("[TOKEN_SYMBOL_ARITHMETIC_MULTIPLY ");
                break;
                case '/':
                    printf("[TOKEN_SYMBOL_ARITHMETIC_DIVIDE ");
                break;
                case '%':
                    printf("[TOKEN_SYMBOL_ARITHMETIC_MOD ");
                break;
                case '^':
                    printf("[TOKEN_SYMBOL_CARET ");
                break;
                case '(':
                    printf("[TOKEN_SYMBOL_LEFT_PAREN ");
                break;
                case ')':
                    printf("[TOKEN_SYMBOL_RIGHT_PAREN ");
                break;
                default:
                    printf("[TOKEN_ERROR ");
                break;
            }
        }


        printf("ln:%d col:%d pos:%d]: %c\n", ln, col, pos, c);

        pos++;
        col++;
    }

    free(source);
    return tokens;
}