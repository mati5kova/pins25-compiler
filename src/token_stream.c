//
// Created by Matevž Kovačič on 27. 7. 25.
//

#include <stdlib.h>
#include "../include/token_stream.h"

TokenStream* createTokenStream(Token** tokens, const int numOfTokens) {
    TokenStream* ts = malloc(sizeof(TokenStream));
    if (!ts) {
        cleanupSourceBuffer();
        cleanupTokens(tokens);
        return NULL;
    }

    ts->tokens = tokens;
    ts->numOfTokens = numOfTokens;
    ts->index = 0;

    return ts;
}

Token* consumeToken(TokenStream* ts) {
    if (ts->index >= ts->numOfTokens - 1) {
        return (ts->tokens)[ts->numOfTokens - 1];
    }

    return (ts->tokens)[ts->index++];
}

Token* peekToken(const TokenStream* ts) {
    const int i = ts->index < ts->numOfTokens ? ts->index : ts->numOfTokens - 1;

    return (ts->tokens)[i];
}

bool checkToken(TokenStream* ts, const TokenType expectedType) {
    const Token* peeked = peekToken(ts);
    if (peeked->type == expectedType)
    {
        consumeToken(ts);
        return true;
    } else
    {
        return false;
    }
}

Token* prevCheckedToken(const TokenStream* ts) {
    const int i = (ts->index > 0) ? ts->index - 1 : 0;

    return ts->tokens[i];
}

Token* currentToken(const TokenStream* ts) {
    return (ts->tokens)[ts->index];
}

Token* rewindToken(TokenStream* ts) {
    if (ts->index > 0) {
        ts->index = ts->index - 1;
    }
    return (ts->tokens)[ts->index];
}

void freeTokenStream(TokenStream* ts) {
    free(ts);
}