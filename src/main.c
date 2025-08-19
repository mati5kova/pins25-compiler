#include <stdio.h>
#include <stdlib.h>

#include "../include/compiler_data.h"
#include "../include/lexer.h"
#include "../include/options.h"
#include "../include/parser.h"
#include "../include/ast.h"

// da je izpis kot npr. parsing failed izpise vedno na dnu v rdeci
#define ERROR_STDOUT(msg) fprintf(stdout, "%s%s%s\n", "\x1b[31m", msg, "\x1b[0m");

int main(const int argc, char* argv[]) {
    CompilerData* compData = createCompilerData();
    if (!compData)
    {
        fprintf(stderr, "Failed to create compiler data\n");
        return EXIT_FAILURE;
    }

    init_options(&(compData->opts));

    if (parse_args(argc, argv, &(compData->opts)) < 0) {
        print_usage(argv[0]);

        goto cleanup;
    } else if ((compData->opts).n_inputs == 0) {
        fprintf(stderr, "Error: no file to compile\n");
        print_usage(argv[0]);

        goto cleanup;
    } else if ((compData->opts).n_inputs > 1) {
        fprintf(stderr, "Error: too many files to compile\n");
        print_usage(argv[0]);

        goto cleanup;
    } else if ((compData->opts).n_inputs == 1) {
        compData->inputFile = fopen((compData->opts).input_files[0], "r");
        if (!(compData->inputFile)) {
            perror("Failed to open input file");
            goto cleanup;
        }

        compData->inputFileName = (compData->opts).input_files[0];
    }

    // LEKSIKALNA ANALIZA
    {
        compData->tokens = tokenize(compData);

        if (!(compData->tokens)) { // najverjetneje kaksen NULL malloc
            fprintf(stderr, "Error occured during lexical analysis of '%s'\n", (compData->opts).input_files[0]);
            goto cleanup;
        }

        if (!(compData->lexOK)) // leksikalna analiza ni bila uspesna
        {
            ERROR_STDOUT("Lexing failed");
            goto cleanup;
        }

        if ((compData->opts).list_tokens_all) {
            if (!printTokens(compData,true))
            {
                goto cleanup;
            }
        }
    }

    // USTVARI SE TOKEN STREAM
    {
        compData->ts = createTokenStream(compData->tokens, compData->tokenCount);
        if (!(compData->ts)) {
            ERROR_STDOUT("Error creating token stream");
            goto cleanup;
        }
    }

    // SINTAKSNA ANALIZA
    {
        compData->rootASTNode = parse(compData);
        const bool parsingSuccessfull = passedSyntaxAnalysis();

        if (!(compData->rootASTNode) || !parsingSuccessfull)
        {
            ERROR_STDOUT("Parsing failed");
            goto cleanup;
        }

        if ((compData->opts).ast)
        {
            printAST(compData->rootASTNode, 0);
        }
    }

    // cleanup ce je bilo vse vredu
    destroyCompilerData(compData);
    return EXIT_SUCCESS;

cleanup:
    destroyCompilerData(compData);
    return EXIT_FAILURE;
}
