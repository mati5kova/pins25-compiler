//
// Created by Matevz on 03/07/2025.
//

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/error_utils.h"
#include "../include/lexer.h"

#define INITIAL_TOKEN_COUNT 200       // zacetno stevilo tokenov
#define TOKEN_COUNT_INCREMENT 100     // increment za stevilo tokenov

#define INITIAL_BUFFER_SIZE 500       // zacetna velikost source bufferja
#define BUFFER_SIZE_INCREMENT 250     // increment za source buffer

#define MAX_IDENTIFIER_LENGTH 64      // maksimalna dolzina za imena spremenljivk (63 znakov + \0)

bool passedLexicalAnalysis = true;    // bool ali so bile v fazi lexikalne analize zaznane kaksne napake

int numberOfInputChars = 0;           // stevilo znakov v vhodni datoteki da vemo do kam loop-amo v `tokenize`

int tokenCount = 0;                   // stevilo tokenov v tabeli tokenov

char* source;                         // kazalec na buffer z vsebino vhodne datoteke

// stevilo rezerviranih besed jezika
const short numberOfReservedKeywords = 10;
const char* reservedKeywords[] = {"fun", "var", "if", "then", "else", "while", "do", "let", "in", "end"};

// `skipUntilLineEnd` bere znake do konca vrstice in vrne stevilo prebranih znakov
int skipUntilLineEnd(const char* src) {
    int counter = 0;
    while (src[counter] != '\n' && src[counter] != '\0') {
        counter++;
    }

    return counter;
}

// `retrieveTokenCount` vrne stevilo tokenov ki jih je ustvaril lexikalni strol
int retrieveTokenCount() { return tokenCount; }

// `isLexicallValid` vrne true ce ni bilo zaznane nobene lexikalne napake, drugace vrne false
bool isLexicallyValid() { return passedLexicalAnalysis; }

// helper funkcija `incPosition` poveca vrednosti vhodnih argumentov za +1
// da ni potrebno vedno pisati: pos++; col++;
void incPosition(int* a, int* b) {
    (*a)++;
    (*b)++;
}

// `cleanupSourceBuffer` sprosti pomnilnik ki ga zavzame source buffer
// pomembno je da se klice ob napaki in pa na koncu main funkcije, saj na source buffer kazejo kazalci znotraj posameznih Tokenov
void cleanupSourceBuffer() {
    free(source);
}

/*
 * `stringifyInputFile` vrne kazalec na tabelo znakov
 * lazje se premikamo po tabeli znakov kot pa po datoteki zato
 * celotno vsebino vhodne datoteke shranimo v buffer ki ga nato uporabljamo
 * kot source v `tokenize` funkciji
 */
char* stringifyInputFile(FILE* inputFile) {
    rewind(inputFile);
    numberOfInputChars = 0;

    // imamo zacetni buffer size ki ga po potrebi povecujemo
    int currentMaxBufferSIze = INITIAL_BUFFER_SIZE;
    char* buffer = malloc(INITIAL_BUFFER_SIZE * sizeof(char));
    if (!buffer) {
        printf("Out of memory [lexer.c stringifyInputFile]\n");
        exit(EXIT_FAILURE);
    }

    int c;
    while ((c = fgetc(inputFile)) != EOF) {
        if (c == '\r') {
            continue;
        }

        if (numberOfInputChars + 1 >= currentMaxBufferSIze) {
            char* temp = realloc(buffer, currentMaxBufferSIze + BUFFER_SIZE_INCREMENT);
            if (!temp) {
                free(buffer);
                printf("Failed to reallocate buffer [lexer.c stringifyInputFile]\n");
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

/*
 * glavna funkcija lexerja
 * `tokenize` kot argument dobi vhodno datoteko ki jo nato poda funkciji `stringifyInputFile`
 *
 * `tokenize` kot rezultat vrne kazalec na prvi element tabele kazalcev na posamezne Token-e
 */
Token** tokenize(FILE* inputFile, const Options* opts, const char* fileName) {
    int ln = 1;
    int col = 1;
    int pos = 0;

    // stevilo tokenov ki jih trenutni token buffer lahko drzi, po potrebi se poveca
    int maxNumberOfTokensInBuffer = INITIAL_TOKEN_COUNT;

    Token** tokens = malloc(INITIAL_TOKEN_COUNT * sizeof(Token*));
    if (!tokens) {
        printf("Out of memory [lexer.c tokenize]\n");
        exit(EXIT_FAILURE);
    }

    // source je sedaj nasa tabela znakov napolnjena z vsebino vhodne datoteke
    source = stringifyInputFile(inputFile);

    // glavni loop ki gre skozi vse znake source bufferja
    while (source[pos] != '\0' && pos < numberOfInputChars) {
        if (tokenCount + 1 >= maxNumberOfTokensInBuffer) {
            maxNumberOfTokensInBuffer += TOKEN_COUNT_INCREMENT;
            Token** temp = realloc(tokens, maxNumberOfTokensInBuffer * sizeof(Token*));
            if (!temp) {
                fprintf(stderr, "Failed to reallocate tokens array\n");
                cleanupSourceBuffer();
                exit(EXIT_FAILURE);
            }
            tokens = temp;
        }

        const char c = source[pos];

        // ob presledkih se samo premaknemo naprej
        if (c == ' ') {
            incPosition(&pos, &col);
            continue;
        }

        // ob novi vrstici povecamo ln in pos za +1 in nastavimo col na zacetek (=1)
        if (c == '\n') {
            incPosition(&pos, &ln);
            col = 1;
            continue;
        }

        // ce sta trenutni in naslednji znak enaka `/` klicemo `skipUntilLineEnd` in rezultat pristejemo poziciji
        if (c == '/' && source[pos + 1] == '/') {
            pos += skipUntilLineEnd(&(source[pos]));
            continue;
        }

        // ustvarimo nov Token
        Token* newToken = malloc(sizeof(Token));
        if (!newToken) {
            printf("Out of memory [lexer.c tokenize]\n");
            cleanupSourceBuffer();
            exit(EXIT_FAILURE);
        }
        newToken->start = &(source[pos]); // zacetek lexema je trenutni znak
        newToken->location = malloc(sizeof(InFileLocation));
        if (!(newToken->location)) {
            printf("Out of memory [lexer.c tokenize]\n");
            cleanupSourceBuffer();
            free(newToken);
            exit(EXIT_FAILURE);
        }
        newToken->location->col = col;
        newToken->location->ln = ln;
        newToken->location->pos = pos;

        // cela stevila
        if (isdigit(c)) {
            bool isValidNumber = true;
            const int firstDigit = c - '0';

            // dejanska vrednost celostevilske konstante ki jo beremo
            int number = 0;

            // stevilo stevk v stevilu
            int lexemLength = 0;

            if (firstDigit == 0 && isdigit(source[pos + 1])) {
                isValidNumber = false;
                newToken->type = TOKEN_ERROR;
            }

            // beremo dokler so stevke
            while (isdigit(source[pos]) && source[pos] != '\0' && source[pos] != '\n' && source[pos] != EOF) {
                number = number * 10 + (source[pos] - '0');
                lexemLength++;
                incPosition(&pos, &col);
            }

            newToken->length = lexemLength;

            if (isValidNumber) {
                newToken->type = TOKEN_CONSTANT_INT;
            } else {
                passedLexicalAnalysis = false;
                printError(fileName, "invalid numeric constant", ln, col, pos, newToken->start, lexemLength);
            }

            tokens[tokenCount++] = newToken;
            continue;
        }

        // znakovna konstanta
        if (c == '\'') {
            // moramo premakniti za eno naprej da se ['] ne steje v lexem
            if (pos + 1 < numberOfInputChars) {
                newToken->start = &(source[pos + 1]);
            }

            char character[4];
            short int characterLength = 0;
            incPosition(&pos, &col);
            bool charConstTooLong = false;

            while (source[pos] != '\0' && source[pos] != '\'') {
                if (characterLength >= 4) {
                    charConstTooLong = true;
                }

                if (!charConstTooLong) {
                    character[characterLength++] = source[pos];
                }

                incPosition(&pos, &col);
            }
            incPosition(&pos, &col);

            newToken->length = characterLength;

            if (charConstTooLong) {
                newToken->type = TOKEN_ERROR;
                passedLexicalAnalysis = false;
                printError(fileName, "invalid character constant", ln, col, pos, newToken->start, characterLength);
            } else {
                character[characterLength] = '\0';
                newToken->length = characterLength;

                if (character[0] == '\\') {
                    if ((character[1] == 'n' || character[1] == '\\' || character[1] == '\'') && characterLength == 2) {
                        // [\n]   [\']  [\\]
                        newToken->type = TOKEN_CONSTANT_CHAR;
                    } else if ((ishexnumber(character[1]) && ishexnumber(character[2])) && characterLength == 3) {
                        // [\ff]
                        newToken->type = TOKEN_CONSTANT_CHAR;
                    } else {
                        newToken->type = TOKEN_ERROR;
                        passedLexicalAnalysis = false;
                        printError(fileName, "invalid character constant", ln, col, pos, newToken->start, characterLength);
                    }
                } else if (characterLength == 1) {
                    // [a], [b], ...
                    if (isalpha(character[0])) {
                        newToken->type = TOKEN_CONSTANT_CHAR;
                    }
                } else {
                    passedLexicalAnalysis = false;
                    newToken->type = TOKEN_ERROR;
                    printError(fileName, "invalid character constant", ln, col, pos, newToken->start, characterLength);
                }
            }

            tokens[tokenCount++] = newToken;
            continue;
        }

        // string constant
        if (c == '"') {
            char* strConst = malloc(INITIAL_BUFFER_SIZE * sizeof(char));
            if (!strConst) {
                printf("Out of memory [lexer.c tokenize]\n");
                cleanupSourceBuffer();
                exit(EXIT_FAILURE);
            }
            incPosition(&pos, &col);

            int strConstLen = 0;
            int currStrConstSize = INITIAL_BUFFER_SIZE;

            bool firstChar = true;
            while (!(source[pos] == '"' && !firstChar && source[pos - 1] != '\\')) {
                if (strConstLen + 1 >= currStrConstSize) {
                    char* temp = realloc(strConst, currStrConstSize + BUFFER_SIZE_INCREMENT);
                    if (!temp) {
                        free(strConst);
                        printf("Failed to reallocate buffer [lexer.c tokenize]\n");
                        cleanupSourceBuffer();
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

            newToken->type = TOKEN_CONSTANT_STRING;
            newToken->length = strConstLen;

            free(strConst);

            tokens[tokenCount++] = newToken;
            continue;
        }

        // keywords and identifiers
        if (isalpha(c) || c == '_') {
            char* keyIdentBuffer = malloc(MAX_IDENTIFIER_LENGTH * sizeof(char));
            if (!keyIdentBuffer) {
                printf("Out of memory [lexer.c tokenize]\n");
                cleanupSourceBuffer();
                exit(EXIT_FAILURE);
            }

            bool isKeyword = false;
            int keyIdentLen = 0;

            while (isalnum(source[pos]) && source[pos] != '\0' && source[pos] != '\n') {
                keyIdentBuffer[keyIdentLen++] = source[pos];

                incPosition(&pos, &col);
            }

            keyIdentBuffer[keyIdentLen] = '\0';

            for (int i = 0; i < numberOfReservedKeywords; i++) {
                if (strcmp(keyIdentBuffer, reservedKeywords[i]) == 0) {
                    isKeyword = true;
                }
            }

            newToken->length = keyIdentLen;

            if (isKeyword) {

                if (strcmp(keyIdentBuffer, "fun") == 0) {
                    newToken->type = TOKEN_KEYWORD_FUN;
                } else if (strcmp(keyIdentBuffer, "var") == 0) {
                    newToken->type = TOKEN_KEYWORD_VAR;
                } else if (strcmp(keyIdentBuffer, "if") == 0) {
                    newToken->type = TOKEN_KEYWORD_IF;
                } else if (strcmp(keyIdentBuffer, "then") == 0) {
                    newToken->type = TOKEN_KEYWORD_THEN;
                } else if (strcmp(keyIdentBuffer, "else") == 0) {
                    newToken->type = TOKEN_KEYWORD_ELSE;
                } else if (strcmp(keyIdentBuffer, "while") == 0) {
                    newToken->type = TOKEN_KEYWORD_WHILE;
                } else if (strcmp(keyIdentBuffer, "do") == 0) {
                    newToken->type = TOKEN_KEYWORD_DO;
                } else if (strcmp(keyIdentBuffer, "let") == 0) {
                  newToken->type = TOKEN_KEYWORD_LET;
                } else if (strcmp(keyIdentBuffer, "in") == 0) {
                    newToken->type = TOKEN_KEYWORD_IN;
                } else if (strcmp(keyIdentBuffer, "end") == 0) {
                    newToken->type = TOKEN_KEYWORD_END;
                }
            }

           if (!isKeyword) {
                newToken->type = TOKEN_IDENTIFIER;
            }

            free(keyIdentBuffer);

            tokens[tokenCount++] = newToken;
            continue;
        }

        newToken->length = 1; // default vrednost, po potrebi damo na 2
        if (c == '&') {
            if (source[pos + 1] == '&') {
                newToken->type = TOKEN_SYMBOL_LOGICAL_AND;
                incPosition(&pos, &col);
                newToken->length = 2;
            } else {
                printError(fileName, "invalid logical operator", ln, col, pos, newToken->start, 2);
                passedLexicalAnalysis = false;
                newToken->type = TOKEN_ERROR;
            }
        } else if (c == '|') {
            if (source[pos + 1] == '|') {
                incPosition(&pos, &col);
                newToken->type = TOKEN_SYMBOL_LOGICAL_OR;
                newToken->length = 2;
            } else {
                printError(fileName, "invalid logical operator", ln, col, pos, newToken->start, 2);
                passedLexicalAnalysis = false;
                newToken->type = TOKEN_ERROR;
            }
        } else if (c == '!') {
            if (source[pos + 1] == '=') {
                incPosition(&pos, &col);
                newToken->type = TOKEN_SYMBOL_LOGICAL_NOT_EQUALS;
                newToken->length = 2;
            } else {
                newToken->type = TOKEN_SYMBOL_LOGICAL_NOT;
            }
        } else if (c == '=') {
            if (source[pos + 1] == '=') {
                incPosition(&pos, &col);
                newToken->type = TOKEN_SYMBOL_LOGICAL_EQUALS;
                newToken->length = 2;
            } else {
                newToken->type = TOKEN_SYMBOL_ASSIGN;
            }
        } else if (c == '>') {
            if (source[pos + 1] == '=') {
                incPosition(&pos, &col);
                newToken->type = TOKEN_SYMBOL_LOGICAL_GREATER_OR_EQUALS;
                newToken->length = 2;
            } else {
                newToken->type = TOKEN_SYMBOL_LOGICAL_GREATER;
            }
        } else if (c == '<') {
            if (source[pos + 1] == '=') {
                incPosition(&pos, &col);
                newToken->type = TOKEN_SYMBOL_LOGICAL_LESS_OR_EQUALS;
                newToken->length = 2;
            } else {
                newToken->type = TOKEN_SYMBOL_LOGICAL_LESS;
            }
        } else {
            // za posamezne znake oz. operatorje...
            switch (c) {
                case ',':
                    newToken->type = TOKEN_SYMBOL_COMMA;
                    break;
                case '+':
                    newToken->type = TOKEN_SYMBOL_ARITHMETIC_PLUS;
                    break;
                case '-':
                    newToken->type = TOKEN_SYMBOL_ARITHMETIC_MINUS;
                    break;
                case '*':
                    newToken->type = TOKEN_SYMBOL_ARITHMETIC_MULTIPLY;
                    break;
                case '/':
                    newToken->type = TOKEN_SYMBOL_ARITHMETIC_DIVIDE;
                    break;
                case '%':
                    newToken->type = TOKEN_SYMBOL_ARITHMETIC_MOD;
                    break;
                case '^':
                    newToken->type = TOKEN_SYMBOL_CARET;
                    break;
                case '(':
                    newToken->type = TOKEN_SYMBOL_LEFT_PAREN;
                    break;
                case ')':
                    newToken->type = TOKEN_SYMBOL_RIGHT_PAREN;
                    break;
                default:
                    newToken->type = TOKEN_ERROR;
                    passedLexicalAnalysis = false;
                    printError(fileName, "unknown symbol", ln, col, pos, newToken->start, 1);
                    break;
            }
        }
        incPosition(&pos, &col);
        tokens[tokenCount++] = newToken;
    }

    // ustvarimo zadnji (eof) Token (za potrebe TokenStream funkcij)
    Token* eofToken = malloc(sizeof(Token));
    if (!eofToken) {
        printf("Out of memory [lexer.c tokenize]\n");
        cleanupSourceBuffer();
        exit(EXIT_FAILURE);
    }
    eofToken->type = TOKEN_EOF;
    eofToken->start = NULL;
    eofToken->length = 0;
    eofToken->location = NULL;

    tokens[tokenCount++] = eofToken;

    return tokens;
}

//
// TokenStream funkcije
//
TokenStream* createTokenStream(Token** tokens, const int numOfTokens) {
    TokenStream* ts = malloc(sizeof(TokenStream));
    if (!ts) {
        printf("Out of memory [lexer.c tokenize]\n");
        cleanupSourceBuffer();
        cleanupTokens(numOfTokens, tokens);
        exit(EXIT_FAILURE);
    }

    ts->tokens = tokens;
    ts->numOfTokens = numOfTokens;
    ts->index = 0;

    return ts;
}

Token* nextToken(TokenStream* ts) {
    if (ts->index >= ts->numOfTokens) {
        return (ts->tokens)[ts->numOfTokens - 1];
    }

    return (ts->tokens)[ts->index++];
}

Token* peekToken(const TokenStream* ts) {
    return (ts->tokens)[ts->index];
}

void rewindToken(TokenStream* ts) {
    if (ts->index != 0) {
        ts->index = ts->index - 1;
    }
}

void freeTokenStream(TokenStream* ts) {
    free(ts);
}

void printTokens(Token** tokens) {
    const int numOfTokens = retrieveTokenCount();

    for (int i = 0; i < numOfTokens; i++) {
        const Token* t = tokens[i];

        const char* typeName = "";
        switch (t->type) {
            case TOKEN_CONSTANT_INT:                        typeName = "TOKEN_CONSTANT_INT";                     break;
            case TOKEN_CONSTANT_CHAR:                       typeName = "TOKEN_CONSTANT_CHAR";                    break;
            case TOKEN_CONSTANT_STRING:                     typeName = "TOKEN_CONSTANT_STRING";                  break;
            case TOKEN_SYMBOL_ASSIGN:                       typeName = "TOKEN_SYMBOL_ASSIGN";                    break;
            case TOKEN_SYMBOL_COMMA:                        typeName = "TOKEN_SYMBOL_COMMA";                     break;
            case TOKEN_SYMBOL_LOGICAL_AND:                  typeName = "TOKEN_SYMBOL_LOGICAL_AND";               break;
            case TOKEN_SYMBOL_LOGICAL_OR:                   typeName = "TOKEN_SYMBOL_LOGICAL_OR";                break;
            case TOKEN_SYMBOL_LOGICAL_NOT:                  typeName = "TOKEN_SYMBOL_LOGICAL_NOT";               break;
            case TOKEN_SYMBOL_LOGICAL_EQUALS:               typeName = "TOKEN_SYMBOL_LOGICAL_EQUALS";            break;
            case TOKEN_SYMBOL_LOGICAL_NOT_EQUALS:           typeName = "TOKEN_SYMBOL_LOGICAL_NOT_EQUALS";        break;
            case TOKEN_SYMBOL_LOGICAL_GREATER:              typeName = "TOKEN_SYMBOL_LOGICAL_GREATER";           break;
            case TOKEN_SYMBOL_LOGICAL_LESS:                 typeName = "TOKEN_SYMBOL_LOGICAL_LESS";              break;
            case TOKEN_SYMBOL_LOGICAL_GREATER_OR_EQUALS:    typeName = "TOKEN_SYMBOL_LOGICAL_GREATER_OR_EQUALS"; break;
            case TOKEN_SYMBOL_LOGICAL_LESS_OR_EQUALS:       typeName = "TOKEN_SYMBOL_LOGICAL_LESS_OR_EQUALS";    break;
            case TOKEN_SYMBOL_ARITHMETIC_PLUS:              typeName = "TOKEN_SYMBOL_ARITHMETIC_PLUS";           break;
            case TOKEN_SYMBOL_ARITHMETIC_MINUS:             typeName = "TOKEN_SYMBOL_ARITHMETIC_MINUS";          break;
            case TOKEN_SYMBOL_ARITHMETIC_MULTIPLY:          typeName = "TOKEN_SYMBOL_ARITHMETIC_MULTIPLY";       break;
            case TOKEN_SYMBOL_ARITHMETIC_DIVIDE:            typeName = "TOKEN_SYMBOL_ARITHMETIC_DIVIDE";         break;
            case TOKEN_SYMBOL_ARITHMETIC_MOD:               typeName = "TOKEN_SYMBOL_ARITHMETIC_MOD";            break;
            case TOKEN_SYMBOL_CARET:                        typeName = "TOKEN_SYMBOL_CARET";                     break;
            case TOKEN_SYMBOL_LEFT_PAREN:                   typeName = "TOKEN_SYMBOL_LEFT_PAREN";                break;
            case TOKEN_SYMBOL_RIGHT_PAREN:                  typeName = "TOKEN_SYMBOL_RIGHT_PAREN";               break;
            case TOKEN_IDENTIFIER:                          typeName = "TOKEN_IDENTIFIER";                       break;
            case TOKEN_KEYWORD_FUN:                         typeName = "TOKEN_KEYWORD_FUN";                      break;
            case TOKEN_KEYWORD_VAR:                         typeName = "TOKEN_KEYWORD_VAR";                      break;
            case TOKEN_KEYWORD_IF:                          typeName = "TOKEN_KEYWORD_IF";                       break;
            case TOKEN_KEYWORD_THEN:                        typeName = "TOKEN_KEYWORD_THEN";                     break;
            case TOKEN_KEYWORD_ELSE:                        typeName = "TOKEN_KEYWORD_ELSE";                     break;
            case TOKEN_KEYWORD_WHILE:                       typeName = "TOKEN_KEYWORD_WHILE";                    break;
            case TOKEN_KEYWORD_DO:                          typeName = "TOKEN_KEYWORD_DO";                       break;
            case TOKEN_KEYWORD_LET:                         typeName = "TOKEN_KEYWORD_LET";                      break;
            case TOKEN_KEYWORD_IN:                          typeName = "TOKEN_KEYWORD_IN";                       break;
            case TOKEN_KEYWORD_END:                         typeName = "TOKEN_KEYWORD_END";                      break;
            case TOKEN_ERROR:                               typeName = "TOKEN_ERROR";                            break;
            case TOKEN_EOF:                                 typeName = "TOKEN_EOF";                              break;
            default:                                        typeName = "UNKNOWN_TOKEN";                          break;
        }

        if (t->type == TOKEN_EOF) {
            printf("Token[%3d] TOKEN_EOF", i);
        } else {
            printf("Token[%3d] %-28s \"%.*s\"  (ln:%d, col:%d, pos:%d)\n",
               i,
               typeName,
               t->length,
               t->start,
               t->location->ln,
               t->location->col,
               t->location->pos);
        }
    }
}

void cleanupTokens(const int numOfTokens, Token** tokens) {
    for (int i = 0; i < numOfTokens; i++) {
        Token* t = tokens[i];
        free(t->location);
        free(t);
    }
    free(tokens);
}