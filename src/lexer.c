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

#define INITIAL_TOKEN_COUNT 100                         // zacetno stevilo tokenov

#define INITIAL_BUFFER_SIZE 500                         // zacetna velikost source bufferja

#define MAX_IDENTIFIER_LENGTH 64                        // maksimalna dolzina za imena spremenljivk (63 znakov + \0)

#define PRINT_HELP_LEXER_TITLE "PINS'25 LEXICAL RULES"  // title za --help za lexikalno analizo

bool passedLexicalAnalysis = true;                      // bool ali so bile v fazi lexikalne analize zaznane kaksne napake

int numberOfInputChars = 0;                             // stevilo znakov v vhodni datoteki da vemo do kam loop-amo v `tokenize`

int tokenCount = 0;                                     // stevilo tokenov v tabeli tokenov

char* source;                                           // kazalec na buffer z vsebino vhodne datoteke

const short numberOfReservedKeywords = 10; // stevilo rezerviranih besed jezika
const char* reservedKeywords[] = {"fun", "var", "if", "then", "else", "while", "do", "let", "in", "end"}; // rezervirane besede

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
        return NULL;
    }

    int c;
    while ((c = fgetc(inputFile)) != EOF) {
        if (c == '\r') {
            continue;
        }

        if (numberOfInputChars + 1 >= currentMaxBufferSIze) {
            char* temp = realloc(buffer, currentMaxBufferSIze * 2);
            if (!temp) {
                free(buffer);
                return NULL;
            }
            buffer = temp;

            currentMaxBufferSIze *= 2;
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
        return NULL;
    }

    // source je sedaj nasa tabela znakov napolnjena z vsebino vhodne datoteke
    source = stringifyInputFile(inputFile);
    if (!source) {
        // ce je bila napaka pri malloc-u v `stringifyInputFile`
        cleanupTokens(tokens);
        return NULL;
    }

    // glavni loop ki gre skozi vse znake source bufferja
    while (source[pos] != '\0' && pos < numberOfInputChars) {
        if (tokenCount + 1 >= maxNumberOfTokensInBuffer) {
            maxNumberOfTokensInBuffer *= 2;
            Token** temp = realloc(tokens, maxNumberOfTokensInBuffer * sizeof(Token*));
            if (!temp) {
                cleanupTokens(tokens);
                cleanupSourceBuffer();
                return NULL;
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
        Token* newToken = createToken(TOKEN_EOF, &(source[pos]), 0, ln, col, pos);
        if (!newToken) {
            cleanupTokens(tokens);
            cleanupSourceBuffer();
            return NULL;
        }

        // cela stevila
        if (isdigit(c)) {
            bool isValidNumber = true;
            const int firstDigit = c - '0';

            // dejanska vrednost celostevilske konstante ki jo beremo
            int number = 0;

            // stevilo stevk v stevilu
            int lexemLength = 0;

            bool hasLeadingZeros = false;
            if (firstDigit == 0 && isdigit(source[pos + 1])) {
                isValidNumber = false;
                newToken->type = TOKEN_ERROR;
                hasLeadingZeros = true;
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
                printLexerError(fileName, "invalid numeric constant", ln, col, pos, newToken->start, lexemLength);
                if (hasLeadingZeros && opts->verbose) {
                    printVerboseInfo("numeric constant has leading zeros");
                }
                if (opts->help) {
                    printHelp(PRINT_HELP_LEXER_TITLE, "Numeric constants consist of non-empty base 10 digits with optional leading +/-.\n"
                                                      "Leading zeros are not allowed.");
                }
            }

            tokens[tokenCount++] = newToken;
            continue;
        }

        // znakovna konstanta
        if (c == '\'') {
            incPosition(&pos, &col);

            bool charConstTooLong = false;
            bool isValidCharConstant = true;

            char character[4];
            short int characterLength = 0;

            while (source[pos] != '\0' && source[pos] != '\'') {
                if (characterLength >= 4) {
                    charConstTooLong = true;
                    isValidCharConstant = false;
                }

                if (!charConstTooLong) {
                    character[characterLength] = source[pos];
                }
                characterLength++;
                incPosition(&pos, &col);
            }
            incPosition(&pos, &col);

            newToken->length = characterLength + 2;

            if (charConstTooLong) {
                newToken->type = TOKEN_ERROR;
                passedLexicalAnalysis = false;
                printLexerError(fileName, "invalid character constant", ln, col, pos, newToken->start, newToken->length);
                if (opts->verbose) {
                    printVerboseInfo("character constant is too long");
                }
            } else {
                if (character[0] == '\\') {
                    if ((character[1] == 'n' || character[1] == '\\' || character[1] == '\'') && characterLength == 2) {
                        // [\n]   [\']  [\\]
                        newToken->type = TOKEN_CONSTANT_CHAR;
                    } else if (isxdigit(character[1]) && characterLength == 3) {
                        // [\f?]
                        if (isxdigit(character[2])) {
                            // [\ff]
                            newToken->type = TOKEN_CONSTANT_CHAR;
                        } else {
                            newToken->type = TOKEN_ERROR;
                            passedLexicalAnalysis = false;
                            isValidCharConstant = false;
                            printLexerError(fileName, "invalid character constant", ln, col, pos, newToken->start, newToken->length);
                            if (opts->verbose) {
                                printVerboseInfo("invalid hexadecimal representation");
                            }
                        }
                    } else {
                        newToken->type = TOKEN_ERROR;
                        passedLexicalAnalysis = false;
                        isValidCharConstant = false;
                        printLexerError(fileName, "invalid character constant", ln, col, pos, newToken->start, newToken->length);
                        if (opts->verbose) {
                            printVerboseInfo("invalid combination of characters after '\\'");
                        }
                    }
                } else if (characterLength == 1) {
                    // [a], [b], ...
                    if (isalpha(character[0])) {
                        newToken->type = TOKEN_CONSTANT_CHAR;
                    }
                } else {
                    passedLexicalAnalysis = false;
                    isValidCharConstant = false;
                    newToken->type = TOKEN_ERROR;
                    printLexerError(fileName, "invalid character constant", ln, col, pos, newToken->start, newToken->length);
                }
            }

            if (!isValidCharConstant && opts->help) {
                printHelp(PRINT_HELP_LEXER_TITLE,
                                "Character constants consist of a single character enclosed in single quotes (').\n"
                                     "Valid forms are:\n"
                                     "  - A printable ASCII character (code 32…126).\n"
                                     "  - A simple escape: \\n, \\\', or \\\\\n"
                                     "  - A hex escape: \\XX where X is a hex digit (0…9, a…f)"
                                     );
            }

            tokens[tokenCount++] = newToken;
            continue;
        }

        // string constant
        if (c == '"') {

            int len = 0;
            bool firstChar = true;
            bool terminated = false;

            bool hasInvalidEscape = false;
            bool hasInvalidHex = false;

            while (source[pos] != '\0') {
                // string se spana cez vec vrstic kar ni dovoljeno
                if (source[pos] == '\n') {
                    terminated = false;
                    break;
                }

                if (source[pos] == '\\') {
                    // simple escapes \n, \\, \", \'
                    if (pos + 1 < numberOfInputChars && (source[pos + 1] == 'n' || source[pos + 1] == '\\' || source[pos + 1] == '"' || source[pos + 1] == '\'')) {
                        // vredu
                        incPosition(&pos, &col);
                        len++;
                    } else if (pos + 2 < numberOfInputChars && isxdigit(source[pos + 1])) {
                        if (isxdigit(source[pos + 2])) {
                            // vredu
                        } else {
                            hasInvalidHex = true;
                        }
                    } else {
                        // npr. \q
                        hasInvalidEscape = true;
                    }
                }

                int backslashCount = 0;
                for (int i = pos - 1; i >= 0 && source[i] == '\\'; --i) {
                    backslashCount++;
                }

                if (source[pos] == '"' && (backslashCount % 2) == 0 && !firstChar) {
                    terminated = true;
                    len++; // odstrani ce noces zadnjega " v lexemu + bool firstChar
                    break;
                }

                firstChar = false;
                incPosition(&pos, &col);
                len++;
            }

            newToken->length = len;

            if (hasInvalidEscape || !terminated || hasInvalidHex) {
                passedLexicalAnalysis = false;
            }

            if (hasInvalidEscape || !terminated || hasInvalidHex) {
                newToken->type = TOKEN_ERROR;
                printLexerError(fileName,"invalid string constant",ln,col, pos, newToken->start,len);

                if (opts->verbose) {
                    if (hasInvalidEscape) {
                        printVerboseInfo("invalid presentation of backslash \\ character");
                    } else if (hasInvalidHex) {
                        printVerboseInfo("invalid hexadecimal representation");
                    } else if (!terminated) {
                        printVerboseInfo("unterminated string constant");
                    }
                }

                if (opts->help) {
                    printHelp(PRINT_HELP_LEXER_TITLE,
                                "String constants consist zero or more characters enclosed in double quotes (\").\n"
                                     "Valid forms are:\n"
                                     "  - Printable ASCII characters (code 32…126).\n"
                                     "  - Simple escapes: \\n, \\\", or \\\\\n"
                                     "  - Hex escapes: \\XX where X is a hex digit (0…9, a…f)"
                                );
                }
            } else {
                newToken->type = TOKEN_CONSTANT_STRING;
            }

            incPosition(&pos, &col);

            tokens[tokenCount++] = newToken;
            continue;
        }

        // keywords and identifiers
        if (isalpha(c) || c == '_') {
            char* keyIdentBuffer = malloc(MAX_IDENTIFIER_LENGTH * sizeof(char));
            if (!keyIdentBuffer) {
                free(newToken->location);
                free(newToken);
                cleanupTokens(tokens);
                cleanupSourceBuffer();
                return NULL;
            }

            bool isKeyword = false;
            int keyIdentLen = 0;

            while ((isalnum(source[pos]) || source[pos] == '_') && source[pos] != '\0' && source[pos] != '\n') {
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

        // logicni in nekateri ostali operatorji
        newToken->length = 1; // default vrednost, po potrebi damo na 2
        if (c == '&') {
            if (source[pos + 1] == '&') {
                newToken->type = TOKEN_SYMBOL_LOGICAL_AND;
                incPosition(&pos, &col);
                newToken->length = 2;
            } else {
                printLexerError(fileName, "invalid logical operator", ln, col, pos, newToken->start, 2);
                passedLexicalAnalysis = false;
                newToken->type = TOKEN_ERROR;
            }
        } else if (c == '|') {
            if (source[pos + 1] == '|') {
                incPosition(&pos, &col);
                newToken->type = TOKEN_SYMBOL_LOGICAL_OR;
                newToken->length = 2;
            } else {
                printLexerError(fileName, "invalid logical operator", ln, col, pos, newToken->start, 2);
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
                    incPosition(&pos, &col);
                    newToken->type = TOKEN_ERROR;
                    passedLexicalAnalysis = false;
                    printLexerError(fileName, "unknown symbol", ln, col, pos, newToken->start, 1);
                    printHelp(PRINT_HELP_LEXER_TITLE,"Allowed symbols: = , && || ! == != > < >= <= + - * / % ^ ( )");
                    break;
            }
        }
        incPosition(&pos, &col);
        tokens[tokenCount++] = newToken;
    }

    // ustvarimo zadnji (eof) Token (za potrebe TokenStream funkcij)
    Token* eofToken = createToken(TOKEN_EOF, NULL, 0, -1, -1, -1);
    if (!eofToken) {
        cleanupTokens(tokens);
        cleanupSourceBuffer();
        return NULL;
    }

    tokens[tokenCount++] = eofToken;

    return tokens;
}


Token* createToken(const TokenType type, char* start, const int length, const int ln, const int col, const int pos) {
    Token* newToken = malloc(sizeof(Token));
    if (!newToken) {
        return NULL;
    }
    newToken->type = type;
    newToken->start = start;
    newToken->length = length;

    newToken->location = malloc(sizeof(InFileLocation));
    if (!newToken->location) {
        free(newToken);
        return NULL;
    }
    newToken->location->ln = ln;
    newToken->location->col = col;
    newToken->location->pos = pos;

    return newToken;
}


static const char* tokenTypeToString(TokenType type) {
    switch (type) {
        case TOKEN_CONSTANT_INT:                     return "TOKEN_CONSTANT_INT";
        case TOKEN_CONSTANT_CHAR:                    return "TOKEN_CONSTANT_CHAR";
        case TOKEN_CONSTANT_STRING:                  return "TOKEN_CONSTANT_STRING";
        case TOKEN_SYMBOL_ASSIGN:                    return "TOKEN_SYMBOL_ASSIGN";
        case TOKEN_SYMBOL_COMMA:                     return "TOKEN_SYMBOL_COMMA";
        case TOKEN_SYMBOL_LOGICAL_AND:               return "TOKEN_SYMBOL_LOGICAL_AND";
        case TOKEN_SYMBOL_LOGICAL_OR:                return "TOKEN_SYMBOL_LOGICAL_OR";
        case TOKEN_SYMBOL_LOGICAL_NOT:               return "TOKEN_SYMBOL_LOGICAL_NOT";
        case TOKEN_SYMBOL_LOGICAL_EQUALS:            return "TOKEN_SYMBOL_LOGICAL_EQUALS";
        case TOKEN_SYMBOL_LOGICAL_NOT_EQUALS:        return "TOKEN_SYMBOL_LOGICAL_NOT_EQUALS";
        case TOKEN_SYMBOL_LOGICAL_GREATER:           return "TOKEN_SYMBOL_LOGICAL_GREATER";
        case TOKEN_SYMBOL_LOGICAL_LESS:              return "TOKEN_SYMBOL_LOGICAL_LESS";
        case TOKEN_SYMBOL_LOGICAL_GREATER_OR_EQUALS: return "TOKEN_SYMBOL_LOGICAL_GREATER_OR_EQUALS";
        case TOKEN_SYMBOL_LOGICAL_LESS_OR_EQUALS:    return "TOKEN_SYMBOL_LOGICAL_LESS_OR_EQUALS";
        case TOKEN_SYMBOL_ARITHMETIC_PLUS:           return "TOKEN_SYMBOL_ARITHMETIC_PLUS";
        case TOKEN_SYMBOL_ARITHMETIC_MINUS:          return "TOKEN_SYMBOL_ARITHMETIC_MINUS";
        case TOKEN_SYMBOL_ARITHMETIC_MULTIPLY:       return "TOKEN_SYMBOL_ARITHMETIC_MULTIPLY";
        case TOKEN_SYMBOL_ARITHMETIC_DIVIDE:         return "TOKEN_SYMBOL_ARITHMETIC_DIVIDE";
        case TOKEN_SYMBOL_ARITHMETIC_MOD:            return "TOKEN_SYMBOL_ARITHMETIC_MOD";
        case TOKEN_SYMBOL_CARET:                     return "TOKEN_SYMBOL_CARET";
        case TOKEN_SYMBOL_LEFT_PAREN:                return "TOKEN_SYMBOL_LEFT_PAREN";
        case TOKEN_SYMBOL_RIGHT_PAREN:               return "TOKEN_SYMBOL_RIGHT_PAREN";
        case TOKEN_IDENTIFIER:                       return "TOKEN_IDENTIFIER";
        case TOKEN_KEYWORD_FUN:                      return "TOKEN_KEYWORD_FUN";
        case TOKEN_KEYWORD_VAR:                      return "TOKEN_KEYWORD_VAR";
        case TOKEN_KEYWORD_IF:                       return "TOKEN_KEYWORD_IF";
        case TOKEN_KEYWORD_THEN:                     return "TOKEN_KEYWORD_THEN";
        case TOKEN_KEYWORD_ELSE:                     return "TOKEN_KEYWORD_ELSE";
        case TOKEN_KEYWORD_WHILE:                    return "TOKEN_KEYWORD_WHILE";
        case TOKEN_KEYWORD_DO:                       return "TOKEN_KEYWORD_DO";
        case TOKEN_KEYWORD_LET:                      return "TOKEN_KEYWORD_LET";
        case TOKEN_KEYWORD_IN:                       return "TOKEN_KEYWORD_IN";
        case TOKEN_KEYWORD_END:                      return "TOKEN_KEYWORD_END";
        case TOKEN_ERROR:                            return "TOKEN_ERROR";
        case TOKEN_EOF:                              return "TOKEN_EOF";
        default:                                     return "UNKNOWN_TOKEN";
    }
}

bool printTokens(Token** tokens, const bool outputToFile) {
    const int numOfTokens = retrieveTokenCount();

    FILE* out = outputToFile ? fopen("tokens.txt", "w") : stdout;
    if (outputToFile && !out) {
        fprintf(stderr, "Error opening tokens.txt for writing\n");
        return false;
    }

    char buf[256];
    for (int i = 0; i < numOfTokens; i++) {
        const Token* t = tokens[i];
        const char* typeName = tokenTypeToString(t->type);

        int n = snprintf(buf, sizeof buf, "Token[%3d] %-28s \"%.*s\"  (ln:%d, col:%d, pos:%d)\n",
            i,
            typeName,
            t->length, t->start,
            t->location->ln,
            t->location->col,
            t->location->pos
        );
        if (n < 0) {
            fprintf(stderr, "Formatting error on token %d\n", i);
            return false;
        }

        fputs(buf, out);
    }

    if (outputToFile) {
        fclose(out);
    }

    return true;
}

void cleanupTokens(Token** tokens) {
    const int numOfTokens = retrieveTokenCount();
    for (int i = 0; i < numOfTokens; i++) {
        Token* t = tokens[i];
        free(t->location);
        free(t);
    }
    free(tokens);
}