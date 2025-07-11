//
// Created by Matevz on 8. 7. 25.
//

#include "../include/options.h"
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

static struct option long_opts[] = {
    { "list-tokens-all", no_argument, 0, 'a' },
    { "verbose",         no_argument, 0, 'v' },
    { 0,                 0,           0,  0  }
};

void init_options(Options *opts) {
    opts->list_tokens_all = false;
    opts->verbose = false;
    opts->input_files = NULL;
    opts->n_inputs = 0;
}

int parse_args(const int argc, char **argv, Options *opts) {
    int c, opt_index = 0;
    while ((c = getopt_long(argc, argv, "av", long_opts, &opt_index)) != -1) {
        switch (c) {
            case 'a':
                opts->list_tokens_all = true;
                break;
            case 'v':
                opts->verbose = true;
                break;
            case '?':
            default:
                return -1;
        }
    }
    opts->n_inputs = (size_t)(argc - optind);
    if (opts->n_inputs > 0)
        opts->input_files = &argv[optind];
    return 0;
}

void print_usage(const char *prog) {
    fprintf(stderr,
        "Usage: %s [options] <source-file>\n"
        "Options:\n"
        "  -a, --list-tokens-all   Izpiše vse tokene iz vira\n"
        "  -v, --verbose           Podrobnejši izpis poteka\n",
        prog
    );
}
