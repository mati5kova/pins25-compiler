//
// Created by Matevz on 03/07/2025.
//

#ifndef LEXER_H
#define LEXER_H

#include <stdbool.h>

#include "./token.h"
#include "./compiler_data.h"

struct CompilerData; // forward declaration

// funkcija spremeni vsebino vhodne datoteke "inputFile" v zaporedje tokenov
Token** tokenize(struct CompilerData* compData);

// funkcja ustvari nov Token*
Token* createToken(TokenType type, char* start, int length, int ln, int col, int pos);

// funkcija izpise vse tokene v berljiv obliki
bool printTokens(const struct CompilerData* compData, bool outputToFile);

#endif //LEXER_H
