#include <stdio.h>
#include <stdlib.h>

#include "../include/lexer.h"
#include "../include/options.h"

int main(const int argc, char* argv[]) {
    FILE* in = NULL;
    Options opts;
    init_options(&opts);

    if (parse_args(argc, argv, &opts) < 0) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (opts.n_inputs == 0) {
        fprintf(stderr, "Error: no file to compile\n");
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (opts.verbose) {
        fprintf(stderr, "Verbose mode ON\n");
    }

    if (opts.n_inputs > 1) {
        fprintf(stderr, "Error: too many files to compile\n");
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (opts.n_inputs == 1) {
        in = fopen(opts.input_files[0], "r");
        if (!in) {
            printf("Error opening input file '%s'\n", opts.input_files[0]);
            return EXIT_FAILURE;
        }
    }

    Token** tokens = tokenize(in);
    const int numOfTokens = retrieveTokenCount();

    if (opts.list_tokens_all) {
        printTokens(tokens);
    }

    // cleanup
    fclose(in);
    cleanupSourceBuffer();
    cleanupTokens(numOfTokens, tokens);

    return EXIT_SUCCESS;
}
