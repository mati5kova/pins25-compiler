//
// Created by Matevž Kovačič on 9. 7. 25.
//

#ifndef ERROR_UTILS_H
#define ERROR_UTILS_H

#include "token.h"

// funkcija za enotni izpis napak
void printLexerError(const char* fileName, const char* errorMsg, int line, int column, int position, const char* lexemStart, int length);

// funkcija za izpis dodatnih informacij ob opts.verbose
void printVerboseInfo(char* additionalErrorMsg);

// funkcija za izpis pomoci ob opts.help
void printHelp(char* title, char* msg);

// funkcija za enotni izpis napak pri sintaksni analizi
void printSyntaxError(const char* fileName, const char* errorMsg, const Token* errToken);


#endif //ERROR_UTILS_H
