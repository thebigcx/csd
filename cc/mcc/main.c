#include "mcc.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

FILE *input_file = NULL, *output_file = NULL;

void cleanup()
{
    fclose(input_file);
    fclose(output_file);
}

void usage()
{
    printf("usage: mcc <input> -o <output>\n");
    exit(-1);
}

FILE *xfopen(const char *path, const char *acc)
{
    FILE *f = fopen(path, acc);
    if (!f) {
        printf("mcc: %s: %s\n", path, strerror(errno));
        exit(-1);
    }

    return f;
}

int main(int argc, char **argv)
{
    char *input = NULL;
    char *output = NULL;

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-o")) {
            output = strdup(argv[++i]);
        } else {
            input = strdup(argv[i]);
        }
    }

    if (!input)
        usage();

    // Change suffix
    if (!output) {
        output = strdup(input);
        strcpy(strrchr(output, '.') + 1, "s");
    }

    atexit(cleanup);

    input_file = xfopen(input, "r");
    output_file = xfopen(output, "w+");

    lex_file(input_file);

    char *t;
    while (t = token())
        printf("%s\n", t);

    free(input);
    free(output);
    return 0;
}