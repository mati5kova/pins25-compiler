//
// Created by Matevz on 03/07/2025.
//
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "../include/lexer.h"

#include <ctype.h>

#define INITIAL_TOKEN_COUNT 100
#define TOKEN_COUNT_INCREMENT 25

#define INITIAL_BUFFER_SIZE 500
#define BUFFER_SIZE_INCREMENT 250

#define MAX_IDENTIFIER_LENGTH 64

uint32_t numberOfInputChars = 0;

uint32_t tokenCount = 0;

short int numberOfReservedKeywords = 10;
char* reservedKeywords[] = {"fun", "var", "if", "then", "else", "while", "do", "let", "in", "end"};
char* reservedKeywordsTokens[] = {"TOKEN_KEYWORD_FUN", "TOKEN_KEYWORD_VAR", "TOKEN_KEYWORD_IF", "TOKEN_KEYWORD_THEN", "TOKEN_KEYWORD_ELSE", "TOKEN_KEYWORD_WHILE", "TOKEN_KEYWORD_DO", "TOKEN_KEYWORD_LET", "TOKEN_KEYWORD_IN", "TOKEN_KEYWORD_END"};

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

void incPosition(uint32_t* a, uint32_t* b) {
    (*a)++;
    (*b)++;
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

    /*
    Token** tokens = malloc(INITIAL_TOKEN_COUNT * sizeof(Token*));
    if (!tokens) {
        printf("Out of memory [lexer.c tokenize]\n");
        exit(EXIT_FAILURE);
    }*/


    char* source = stringifyInputFile(inputFile);

    while (source[pos] != '\0' && pos < numberOfInputChars) {
        const char c = source[pos];

        if (c == ' ') {
            incPosition(&pos, &col);
            continue;
        }

        // handlamo novo vrstico
        if (c == '\n') {
            incPosition(&pos, &ln);
            col = 1;
            printf("\n");
            continue;
        }

        // preverimo za komentar
        if (c == '/' && source[pos + 1] == '/') {
            pos += skipUntilLineEnd(&(source[pos]));
            continue;
        }

        // cela števila
        if (isdigit(c)) {
            bool isValidNumber = true;
            const int digit = c - '0';
            uint32_t number = 0;

            if (digit == 0 && isdigit(source[pos + 1])) {
                isValidNumber = false;
                printf("[TOKEN_ERROR Invalid number. Numeric constants must start with a non zero digit ");
            }

            while (isdigit(source[pos]) && source[pos] != '\0' && source[pos] != '\n' && source[pos] != EOF) {
                number = number * 10 + (source[pos] - '0');

                incPosition(&pos, &col);
            }

            if (isValidNumber) {
                printf("[TOKEN_CONSTANT_INT %d ", number);
            }
            continue;
        }

        // znak
        if (c == '\'') {
            char character[4];
            short int characterLength = 0;
            incPosition(&pos, &col);

            while (source[pos] != '\0' && source[pos] != '\'') {
                if (characterLength == 4) {
                    printf("[TOKEN_ERROR Invalid character. Character must start and end with [\'] while having length==1\n");
                    free(source);
                    exit(EXIT_FAILURE);
                }

                character[characterLength++] = source[pos];
                incPosition(&pos, &col);
            }
            incPosition(&pos, &col);

            character[characterLength] = '\0';

            if (character[0] == '\\') {
                if ((character[1] == 'n' || character[1] == '\\' || character[1] == '\'') && characterLength == 2) {
                    // [\n]   [\']  [\\]
                    printf("[TOKEN_CONSTANT_CHAR %s]\n", character);
                } else if ((ishexnumber(character[1]) && ishexnumber(character[2])) && characterLength == 3){
                    printf("[TOKEN_CONSTANT_CHAR %s]\n", character);
                } else {
                    printf("[TOKEN_ERROR Invalid character. Character must start and end with '\''\n");
                }
            } else if (characterLength == 1) {
                if (isalpha(character[0])) {
                    printf("[TOKEN_CONSTANT_CHAR %s]\n", character);
                }
            } else {
                printf("[TOKEN_ERROR Invalid character. Character must start and end with '\''\n");
            }
            continue;
        }

        // string constants
        if (c == '"') {
            char* strConst = malloc(INITIAL_BUFFER_SIZE * sizeof(char));
            if (!strConst) {
                printf("Out of memory [lexer.c tokenize]\n");
                free(source);
                exit(EXIT_FAILURE);
            }
            incPosition(&pos, &col);

            int strConstLen = 0;
            uint32_t currStrConstSize = INITIAL_BUFFER_SIZE;

            bool firstChar = true;
            while (!(source[pos] == '"' && !firstChar && source[pos - 1] != '\\')) {

                if (strConstLen + 1 >= currStrConstSize) {
                    char* temp = realloc(strConst, currStrConstSize + BUFFER_SIZE_INCREMENT);
                    if (!temp) {
                        free(strConst);
                        printf("Failed to reallocate buffer\n");
                        free(source);
                        exit(EXIT_FAILURE);
                    }
                    strConst = temp;

                    currStrConstSize += BUFFER_SIZE_INCREMENT;
                }

                strConst[strConstLen++] = source[pos];
                incPosition(&pos, &col);
                firstChar = false;
            }
            strConst[strConstLen] = '\0';
            incPosition(&pos, &col);

            printf("[TOKEN_CONSTANT_STRING %s ]", strConst);

            free(strConst);
            continue;
        }

        // keywords and identifiers
        if (isalpha(c) || c == '_') {
            char* keyIdentBuffer = malloc(MAX_IDENTIFIER_LENGTH * sizeof(char));
            if (!keyIdentBuffer) {
                printf("Out of memory [lexer.c tokenize]\n");
                free(source);
                exit(EXIT_FAILURE);
            }

            bool isKeyword = false;
            int keyIdentLen = 0;

            while (isalnum(source[pos]) && source[pos] != '\0' && source[pos] != '\n') {
                keyIdentBuffer[keyIdentLen++] = source[pos];


                incPosition(&pos, &col);
            }
            incPosition(&pos, &col);

            keyIdentBuffer[keyIdentLen] = '\0';
            TokenType tt = TOKEN_ERROR;

            for (int i = 0; i < numberOfReservedKeywords; i++) {
                if (strcmp(keyIdentBuffer, reservedKeywords[i]) == 0) {
                    isKeyword = true;
                }
            }

            if (isKeyword) {
                if (strcmp(keyIdentBuffer, "fun") == 0) {
                    tt = TOKEN_KEYWORD_FUN;
                } else if (strcmp(keyIdentBuffer, "var") == 0) {
                    tt = TOKEN_KEYWORD_VAR;
                } else if (strcmp(keyIdentBuffer, "if") == 0) {
                    tt = TOKEN_KEYWORD_IF;
                } else if (strcmp(keyIdentBuffer, "then") == 0) {
                    tt = TOKEN_KEYWORD_THEN;
                } else if (strcmp(keyIdentBuffer, "else") == 0) {
                    tt = TOKEN_KEYWORD_ELSE;
                } else if (strcmp(keyIdentBuffer, "while") == 0) {
                    tt = TOKEN_KEYWORD_WHILE;
                } else if (strcmp(keyIdentBuffer, "do") == 0) {
                    tt = TOKEN_KEYWORD_DO;
                } else if (strcmp(keyIdentBuffer, "in") == 0) {
                    tt = TOKEN_KEYWORD_IN;
                } else if (strcmp(keyIdentBuffer, "end") == 0) {
                    tt = TOKEN_KEYWORD_END;
                }
            }


            if (isKeyword) {
                printf("[%s %s]\n", reservedKeywordsTokens[tt - TOKEN_KEYWORD_FUN] , keyIdentBuffer);
            } else {
                printf("[TOKEN_IDENTIFIER %s]\n", keyIdentBuffer);
            }


            free(keyIdentBuffer);
            continue;
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
    return NULL;
}