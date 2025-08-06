#include <stdio.h>
#include <stdlib.h>

#include "../include/lexer.h"
#include "../include/options.h"
#include "../include/parser.h"
#include "../include/ast.h"

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

    Token** tokens = tokenize(in, &opts, opts.input_files[0]);

    fclose(in);

    if (!tokens) {
        fprintf(stderr, "Error occured during lexical analysis of '%s'\n", opts.input_files[0]);
        return EXIT_FAILURE;
    }

    const int numOfTokens = retrieveTokenCount();
    const bool passedLexicalAnalysis = isLexicallyValid();

    if (!passedLexicalAnalysis)
    {
        cleanupSourceBuffer();
        cleanupTokens(tokens);
        return EXIT_FAILURE;
    }

    if (opts.list_tokens_all) {
        if (!printTokens(tokens,true))
        {
            cleanupSourceBuffer();
            cleanupTokens(tokens);
            return EXIT_FAILURE;
        }
    }

    TokenStream* ts = createTokenStream(tokens, numOfTokens);
    if (!ts) {
        fprintf(stderr, "Error creating token stream\n");
        return EXIT_FAILURE;
    }


    ASTNode* rootNode = parse(ts, &opts, opts.input_files[0]);
    const bool parsingSuccessfull = passedSyntaxAnalysis();

    if (!rootNode || !parsingSuccessfull)
    {
        fprintf(stderr, "Parsing failed");

        cleanupSourceBuffer();
        cleanupTokens(tokens);
        freeTokenStream(ts);
        return EXIT_FAILURE;
    }

    if (opts.ast)
    {
        printAST(rootNode, 0);
    }

    // cleanup ce je bilo vse vredu
    cleanupSourceBuffer();
    cleanupTokens(tokens);
    freeTokenStream(ts);
    freeAST(rootNode);

    return EXIT_SUCCESS;
}
