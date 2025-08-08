//
// Created by Matevž Kovačič on 27. 7. 25.
//

#ifndef TOKEN_STREAM_H
#define TOKEN_STREAM_H

#include <stdbool.h>

#include "./token.h"

// wrapper struktura za seznam Token-ov
typedef struct {
    Token** tokens;
    int     numOfTokens;
    int     index;
} TokenStream;

// funkcija ustvari TokenStream iz outputa funkcije `tokenize`
TokenStream* createTokenStream(Token** tokens, int numOfTokens);

// funkcija vrne naslednji token in poveca index za +1
// nikoli ne vrne NULL, ko je na koncu znova in znova vraca EOF token
Token* consumeToken(TokenStream* ts);

// funkcija vrne naslednji token ampak ne poveca index-a za +1
Token* peekToken(const TokenStream* ts);

// funkcija peeka token in primerja s pricakovanim,
// ce token ustreza pricakovanemu ga consuma
bool checkToken(TokenStream* ts, TokenType expectedType);

// funkcija vrne prejsnji token v streamu
// uporablja se npr. za assign tokena v ast node po uspesnem checkToken (ki sprozi consumeToken ob uspesnem checku)
Token* prevCheckedToken(const TokenStream* ts);

// funkcija vrne token s trenutnim indeksom v token streamu
Token* currentToken(const TokenStream* ts);

// funkcija prevrti TokenStream za en Token nazaj
Token* rewindToken(TokenStream* ts);

// funkcija sprosti pomnilnik od TokenStream-a ne pa Token** tokens
void freeTokenStream(TokenStream* ts);


#endif //TOKEN_STREAM_H
