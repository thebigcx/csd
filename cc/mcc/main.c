#include "mcc.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>

FILE *input_file = NULL, *output_file = NULL;

void error(const char *msg, ...)
{
    printf("Error: ");

    va_list arg;
    va_start(arg, msg);

    vprintf(msg, arg);

    va_end(arg);
}

void cleanup()
{
    if (input_file)  fclose(input_file);
    if (output_file) fclose(output_file);
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

    cgfile(output_file);
    while (!eof()) {
        decl(token());
    }

    free(input);
    free(output);
    return 0;
}