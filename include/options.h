//
// Created by Matevz on 8. 7. 25.
//

#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdbool.h>
#include <stddef.h>

// dovoljeni compiler flagi-i
typedef struct {
    bool     list_tokens_all;   // -l, --list-tokens-all
    bool     verbose;           // -v, --verbose
    bool     help;              // -h, --help
    bool     ast;               // -a, --ast
    char   **input_files;       // positional args
    size_t   n_inputs;
} Options;

// inicializacija ne defaultne vrednosti (false, zero, NULL)
void init_options(Options *opts);

// parsamo argc/argv -> v `opts`
// return 0 -> success
// return -1 -> bad flag
int parse_args(int argc, char **argv, Options *opts);

// print help
void print_usage(const char *prog);

#endif
