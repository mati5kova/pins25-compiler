//
// Created by Matevz on 03/07/2025.
//

#ifndef LEXER_H
#define LEXER_H

#include <stdbool.h>
#include <stdio.h>

#include "options.h"

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

    TOKEN_ERROR,                               // neveljaven znak, nedokoncan literal, ...

    TOKEN_EOF                                  // konec vhodne datoteke (ni v uporabi)
} TokenType;

typedef struct {
    int ln;                                    // vrstica
    int col;                                   // stolpec oz. znak v vrstici (1 index based)
    int pos;                                   // zaporedni byte v datoteki
} InFileLocation;

typedef struct {
    TokenType       type;                      // tip tokena
    char*           start;                     // kazalec na zacetek lexema
    int             length;                    // stevilo znakov v lexemu (dolzina)
    InFileLocation* location;                  // lokacija lexema v vhodni datoteki
} Token;

// funkcija vrne stevilo tokenov ki so nastali v fazi lexikalne analize
// uporablja se npr. za for-loop cleanup v main.c
int retrieveTokenCount();

// vrne ustrezno boolean vrednost glede na to, ali je bila lexikalna analiza uspesna
bool isLexicallyValid();

// funkcija spremeni vsebino vhodne datoteke "inputFile" v zaporedje tokenov
Token** tokenize(FILE* inputFile, const Options* opts, const char* fileName);

// funkcja ustvari nov Token*
Token* createToken(TokenType type, char* start, int length, int ln, int col, int pos);

// funkcija izpise vse tokene v berljiv obliki
bool printTokens(Token** tokens, bool outputToFile);

// funkcija sprosti pomnilnik ki ga zasede vsebina vhodne datoteke shranjena kot char*
void cleanupSourceBuffer();

// funkcija sprosti pomnilnik ki ga zasedejo ustvarjeni tokeni
void cleanupTokens(Token** tokens);

#endif //LEXER_H
