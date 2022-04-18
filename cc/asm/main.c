#include "asm.h"

#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

char           *g_infile = NULL;
FILE           *g_in  = NULL;
FILE           *g_out = NULL;
struct tok      g_tok = { 0 };
int             g_mode = 8;
struct forward *g_forwards = NULL;
unsigned int    g_forwardcnt = 0;
unsigned int    g_line = 1;
uint32_t        g_bss = 0;

// Prints the current source line
static char *srcline()
{
    size_t n;
    char *str = NULL;

    fseek(g_in, 0, SEEK_SET);
    for (unsigned int line = g_line; line > 0; line--)
        getline(&str, &n, g_in);

    return str;
}

// Print an error message and exit the program
void error(const char *msg, ...)
{
    printf("\x1b[1;31mError\x1b[39m: %s:%d\x1b[0m: ", g_infile, g_line);

    va_list args;
    va_start(args, msg);
    vprintf(msg, args);
    va_end(args);

    char *line = srcline();
    printf(" %d     %s\n", g_line, line);
    free(line);

    printf("Exiting due to assembly errors\n");
    exit(-1);
}

void general_error(const char *msg, ...)
{
    printf("\x1b[1;31mError\x1b[39m: \x1b[0m");

    va_list args;
    va_start(args, msg);
    vprintf(msg, args);
    va_end(args);

    exit(-1);
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("usage: asm <input>\n");
        return -1;
    }

    g_infile = strdup(argv[1]);
    g_in = fopen(g_infile, "r+");
    if (!g_in)
    {
        printf("%s: %s\n", argv[1], strerror(errno));
        return -1;
    }

    // Insert newline at end if necessary
    fseek(g_in, -1, SEEK_END);
    if (fgetc(g_in) != '\n')
        fputc('\n', g_in);

    fseek(g_in, 0, SEEK_SET);

    char *oname = strdup(argv[1]);
    oname[strlen(oname) - 1] = 'o';

    g_out = fopen(oname, "w+");

    dofile();

    free(oname);
    fclose(g_in);
    fclose(g_out);
    return 0;
}