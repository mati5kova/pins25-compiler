//
// Created by Matevž Kovačič on 9. 7. 25.
//

#ifndef ERROR_UTILS_H
#define ERROR_UTILS_H

// funkcija za enotni izpis napak
void printError(const char* fileName, const char* errorType, int line, int column, int position, const char* lexemStart, int length);

// funkcija za izpis dodatnih informacij ob opts.verbose
void printVerboseInfo(char* additionalErrorMsg);

// funkcija za izpis pomoci ob opts.help
void printHelp(char* title, char* msg);

#endif //ERROR_UTILS_H
