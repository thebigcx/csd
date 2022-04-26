#include "lnk.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>

#include <lnk/bin.h>

FILE *g_in[16] = { NULL };
unsigned int g_incnt = 0;
FILE *g_out = NULL;

// Symbol table
struct sym *g_symtab = NULL;
unsigned int g_symcnt = 0;

// Section table
struct sect *g_sects = NULL;
unsigned int g_sectcnt = 0;

uint64_t g_base = 0; // Base address of file

static int s_bin = 0; // Flat binary output

// TODO: current file
void error(const char *msg, ...)
{
    printf("\x1b[1;31mError:\x1b[0m ");

    va_list args;
    va_start(args, msg);
    vprintf(msg, args);
    va_end(args);

    printf("Exiting due to linker errors\n");
    exit(-1);
}

// Read the contents of a file: returns malloc'd buffer
void *readfile(FILE *f)
{
    // Get size of file
    fseek(f, 0, SEEK_END);
    size_t s = ftell(f);
    fseek(f, 0, SEEK_SET);

    void *buf = malloc(s);
    fread(buf, s, 1, f);
    return buf;
}

void do_normal()
{

}

// Match an S_UNDF symbol with an S_GLOB one
struct sym resolve(struct sym *undf)
{
    for (unsigned int i = 0; i < g_symcnt; i++)
    {
        if (!strcmp(undf->name, g_symtab[i].name) && g_symtab[i].flags & S_GLOB)
            return g_symtab[i];
    }

    error("Undefined symbol '%s'\n", undf->name); // TODO: error
    __builtin_unreachable();
}

// Copy a section to the output file
void write_section(int f, int type)
{
    fseek(g_in[f], 0, SEEK_SET);

    struct bin_main main;
    fread(&main, sizeof(struct bin_main), 1, g_in[f]);

    // If BSS, fill with zeroes
    if (type == SE_BSS)
    {
        while (main.bss--) fputc(0, g_out);
        return;
    }

    // Offset and size of input
    uint64_t offset = sizeof(struct bin_main);
    if (type == SE_DATA) offset += main.txtsz;

    size_t size = type == SE_TEXT ? main.txtsz : main.datsz;

    // Read the input, write it to output
    fseek(g_in[f], offset, SEEK_SET);

    char *data = malloc(size);
    fread(data, size, 1, g_in[f]);

    uint64_t base = ftell(g_out);
    fwrite(data, size, 1, g_out);

    // Add a section to the list
    g_sects = realloc(g_sects, (g_sectcnt + 1) * sizeof(struct sect));
    g_sects[g_sectcnt++] = (struct sect) {
        .type    = type,
        .fileidx = f,
        .base    = base
    };

    free(data);
}

int main(int argc, char **argv)
{
    char *input[16] = { NULL };
    char *output = NULL;
    unsigned int cnt = 0;

    // Parse command line arguments
    for (unsigned int i = 1; i < argc; i++)
    {
        char *arg = argv[i];
        if (*arg == '-')
        {
            switch (*(++arg))
            {
                case 'b': s_bin = 1; break;
                case 'a':
                    arg = argv[++i];
                    g_base = strtoull(arg, NULL, 16);
                    break;
                case 'o':
                    output = strdup(argv[++i]);
                    break;

            }
        }
        else
            input[cnt++] = arg;
    }

    if (!input)
    {
        printf("usage: lnk <input> -b -a <base>\n");
        return -1;
    }

    if (!output)
        strcpy(output, "a.out");

    // Open input files
    g_incnt = cnt;
    while (cnt--)
    {
        g_in[cnt] = fopen(input[cnt], "r");
        if (!g_in[cnt])
        {
            printf("lnk: %s: %s\n", input[cnt], strerror(errno));
            return -1;
        }
    }

    g_out = fopen(output, "w+");

    if (s_bin) do_binary();
    else do_normal();

    for (unsigned int i = 0; i < g_incnt; i++)
        fclose(g_in[i]);

    free(output);
    fclose(g_out);
    return 0;
}