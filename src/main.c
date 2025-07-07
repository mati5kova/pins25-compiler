#include <stdio.h>
#include <stdlib.h>
#include "../include/lexer.h"

int main(const int argc, char* argv[]) {

    FILE* in;
    if (argc == 1) {
        printf("Usage: %s [file to compile]\n", argv[0]);
    } else if (argc == 2) {
        in = fopen(argv[1], "r");
        if (!in) {
            printf("Error opening file [%s]. Check spelling and/or location\n", argv[1]);
            exit(EXIT_FAILURE);
        }
    }



    Token** tokens = tokenize(in);



    // cleanup
    fclose(in);
    for (int i = 0; i < retrieveTokenCount(); i++) {
        free(tokens[i]);
    }
    free(tokens);


    return EXIT_SUCCESS;
}