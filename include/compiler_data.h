//
// Created by Matevž Kovačič on 8. 8. 25.
//

#ifndef COMPILER_DATA_H
#define COMPILER_DATA_H

#include <stdio.h>
#include <stdbool.h>

#include "./options.h"
#include "./token_stream.h"
#include "./token.h"
#include "./ast.h"

struct ASTNode; // forward declaration

typedef struct CompilerData{
    FILE*           inputFile;
    char*           inputFileName;
    Options         opts;
    Token**         tokens;
    TokenStream*    ts;
    struct ASTNode* rootASTNode;

    // LEXER
    char*           source;         // buffer z vsebino vhodne datoteke
    int             sourceLen;      // dolzina bufferja
    int             tokenCount;     // skupno stevilo tokenov
    bool            lexOK;          // true | false glede na to ali passa leksikalno analizo
} CompilerData;

// ustvari struct in ga vrne (v main), nastavi defaultne vrednosti
CompilerData* createCompilerData(void);

// cleanup vseh podatkov ki jih drzi
void destroyCompilerData(CompilerData* comp);

#endif //COMPILER_DATA_H
