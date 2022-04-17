#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include <lnk/bin.h>

FILE *g_in = NULL;
FILE *g_out = NULL;

static uint64_t s_base = 0; // Base address of file
static int s_bin = 0; // Flat binary output

// Read the contents of a file: returns malloc'd buffer
void *readfile(FILE *f)
{
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

void do_binary()
{
    char *buf = readfile(g_in);
    struct bin_main *main = (struct bin_main*)buf;

    fwrite(buf + sizeof(struct bin_main), main->txtsz, 1, g_out);
    fwrite(buf + sizeof(struct bin_main) + main->txtsz, main->datsz, 1, g_out);

    struct rel *rels = (struct rel*)(buf + main->txtrel);
    struct symbol *syms = (struct symbol*)(buf + main->symtab);

    // Do text relocations
    for (unsigned int i = 0; i < (main->datrel - main->txtrel) / sizeof(struct rel); i++)
    {
        uint64_t val = syms[rels[i].sym].value + s_base + rels[i].addend;
        fseek(g_out, rels[i].addr, SEEK_SET);
        fwrite(&val, rels[i].size, 1, g_out);
    }

    free(buf);
}

int main(int argc, char **argv)
{
    char *input = NULL;

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
                    s_base = strtoull(arg, NULL, 16);
                    break;
            }
        }
        else
            input = arg;
    }

    if (!input)
    {
        printf("usage: lnk <input> -b\n");
        return -1;
    }

    // Open input file
    g_in = fopen(input, "r");
    if (!g_in)
    {
        printf("lnk: %s: %s\n", input, strerror(errno));
        return -1;
    }

    // Remove file extension
    char *out = strdup(input);
    char *dot = strrchr(out, '.');
    if (dot) *dot = 0;

    g_out = fopen(out, "w+");
    free(out);

    if (s_bin)
        do_binary();
    else
        do_normal();

    fclose(g_in);
    fclose(g_out);
    return 0;
}