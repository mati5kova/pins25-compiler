//
// Created by Matevž Kovačič on 8. 8. 25.
//

#include <stdio.h>
#include <stdlib.h>

#include "../include/compiler_data.h"

CompilerData* createCompilerData() {
    CompilerData* comp = (CompilerData*) malloc(sizeof(CompilerData));
    if (!comp)
    {
        return NULL;
    }

    comp->inputFile = NULL;
    comp->inputFileName = NULL;
    comp->tokens = NULL;
    comp->ts = NULL;
    comp->rootASTNode = NULL;

    comp->source = NULL;
    comp->sourceLen = 0;
    comp->tokenCount = 0;
    comp->lexOK = true;

    return comp;
}

void destroyCompilerData(CompilerData* comp) {
    if (!comp) { return; }

    if (comp->inputFile)
    {
        const int close = fclose(comp->inputFile);
        if (close != 0)
        {
            perror("Error closing file\n");
        }
    }

    if (comp->tokens)
    {
        for (int i = 0; i < comp->tokenCount; i++) {
            Token* t = (comp->tokens)[i];
            free(t->location);
            free(t);
        }
        free(comp->tokens);
    }

    if (comp->ts)
    {
        freeTokenStream(comp->ts);
    }

    if (comp->rootASTNode)
    {
        freeAST(comp->rootASTNode);
    }

    if (comp->source)
    {
        free(comp->source);
    }

    free(comp);
}