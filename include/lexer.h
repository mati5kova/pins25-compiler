//
// Created by Matevz on 03/07/2025.
//

#ifndef LEXER_H
#define LEXER_H

#include <stdint.h>
#include <stdio.h>

typedef enum {
    TOKEN_CONSTANT_INT,                        // 123
    TOKEN_CONSTANT_CHAR,                       // 'M'
    TOKEN_CONSTANT_STRING,                     // "Hello, World!"

    TOKEN_SYMBOL_ASSIGN,                       // =

    TOKEN_SYMBOL_COMMA,                        // ,

    TOKEN_SYMBOL_LOGICAL_AND,                  // &&
    TOKEN_SYMBOL_LOGICAL_OR,                   // ||
    TOKEN_SYMBOL_LOGICAL_NOT,                  // !
    TOKEN_SYMBOL_LOGICAL_EQUALS,               // ==
    TOKEN_SYMBOL_LOGICAL_NOT_EQUALS,           // !=
    TOKEN_SYMBOL_LOGICAL_GREATER,              // >
    TOKEN_SYMBOL_LOGICAL_LESS,                 // <
    TOKEN_SYMBOL_LOGICAL_GREATER_OR_EQUALS,    // >=
    TOKEN_SYMBOL_LOGICAL_LESS_OR_EQUALS,       // <=

    TOKEN_SYMBOL_ARITHMETIC_PLUS,              // +
    TOKEN_SYMBOL_ARITHMETIC_MINUS,             // -
    TOKEN_SYMBOL_ARITHMETIC_MULTIPLY,          // *
    TOKEN_SYMBOL_ARITHMETIC_DIVIDE,            // /
    TOKEN_SYMBOL_ARITHMETIC_MOD,               // %

    TOKEN_SYMBOL_CARET,                        // ^

    TOKEN_SYMBOL_LEFT_PAREN,                   // (
    TOKEN_SYMBOL_RIGHT_PAREN,                  // )

    TOKEN_IDENTIFIER,                          // var_name

    TOKEN_KEYWORD_FUN,                         // fun
    TOKEN_KEYWORD_VAR,                         // var
    TOKEN_KEYWORD_IF,                          // if
    TOKEN_KEYWORD_THEN,                        // then
    TOKEN_KEYWORD_ELSE,                        // else
    TOKEN_KEYWORD_WHILE,                       // while
    TOKEN_KEYWORD_DO,                          // do
    TOKEN_KEYWORD_LET,                         // let
    TOKEN_KEYWORD_IN,                          // in
    TOKEN_KEYWORD_END,                         // end

    TOKEN_ERROR,                               // neveljaven znak, nedokon"an literal

    TOKEN_EOF                                  // konec vhodne datoteke
} TokenType;

typedef struct {
    uint32_t ln;                               // vrstica
    uint32_t col;                              // stolpec oz. znak v vrstici (1 index based)
    uint32_t pos;                              // zaporedni byte v datoteki
} InFileLocation;

typedef struct {
    TokenType      type;                       // tip tokena
    const char     *start;                     // kazalec na zacetek lexema
    size_t         length;                     // stevilo znakov v lexemu (dolzina)
    InFileLocation location;                   // lokacija lexema v vhodni datoteki
} Token;

uint32_t retrieveTokenCount();

char* stringifyInputFile(FILE* inputFile);

Token** tokenize(FILE* inputFile);

#endif //LEXER_H
