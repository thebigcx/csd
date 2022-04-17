#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include <lnk/bin.h>

FILE *g_in[16] = { NULL };
unsigned int g_incnt = 0;
FILE *g_out = NULL;

static uint64_t s_base = 0; // Base address of file
static int s_bin = 0; // Flat binary output

struct sym
{
    char *name;
    uint64_t val;
    uint8_t flags;

    int index;   // Index in object file
    int fileidx; // File index (for offset stuff when combining objects)
};

#define SE_TEXT 1
#define SE_DATA 2

struct sect
{
    int type;
    int fileidx;
    uint64_t base;
};

// Symbol table
struct sym *g_symtab = NULL;
unsigned int g_symcnt = 0;

// Section table
struct sect *g_sects = NULL;
unsigned int g_sectcnt = 0;

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

// TODO: do data/bss sections as well
void do_binary()
{
    // 1st pass: Extract symbols from each input file and link external definitions together
    for (unsigned int i = 0; i < g_incnt; i++)
    {
        size_t len;

        fseek(g_in[i], 0, SEEK_END);
        len = ftell(g_in[i]);
        fseek(g_in[i], 0, SEEK_SET);

        struct bin_main main;
        fread(&main, sizeof(struct bin_main), 1, g_in[i]);

        char *strs = malloc(len - main.strtab);
        fseek(g_in[i], main.strtab, SEEK_SET);
        fread(strs, len - main.strtab, 1, g_in[i]);

        fseek(g_in[i], main.symtab, SEEK_SET);

        for (unsigned int j = 0; j < main.strtab - main.symtab; j += sizeof(struct symbol))
        {
            struct symbol sym;
            fread(&sym, sizeof(struct symbol), 1, g_in[i]);

            g_symtab = realloc(g_symtab, (g_symcnt + 1) * sizeof(struct sym));
            g_symtab[g_symcnt++] = (struct sym) {
                .name    = strdup(strs + sym.name),
                .flags   = sym.flags,
                .val     = sym.value,
                .index   = j / sizeof(struct symbol),
                .fileidx = i
            };
        }

        free(strs);
    }

    // 2nd pass: Write text sections
    for (unsigned int i = 0; i < g_incnt; i++)
    {
        fseek(g_in[i], 0, SEEK_SET);

        struct bin_main main;
        fread(&main, sizeof(struct bin_main), 1, g_in[i]);

        char *text = malloc(main.txtsz);
        fread(text, main.txtsz, 1, g_in[i]);

        uint64_t base = ftell(g_out);
        fwrite(text, main.txtsz, 1, g_out);

        g_sects = realloc(g_sects, (g_sectcnt + 1) * sizeof(struct sect));
        g_sects[g_sectcnt++] = (struct sect) {
            .type    = SE_TEXT,
            .fileidx = i,
            .base    = base
        };

        free(text);
    }

    // 3rd pass: Do text relocations
    for (unsigned int i = 0; i < g_incnt; i++)
    {
        fseek(g_in[i], 0, SEEK_SET);

        struct bin_main main;
        fread(&main, sizeof(struct bin_main), 1, g_in[i]);

        fseek(g_in[i], main.txtrel, SEEK_SET);

        for (unsigned int j = 0; j < main.datrel - main.txtrel; j += sizeof(struct rel))
        {
            struct rel rel;
            fread(&rel, sizeof(struct rel), 1, g_in[i]);

            // Find symbol (will not fail)
            struct sym sym;
            for (unsigned int k = 0; k < g_symcnt; k++)
            {
                if (g_symtab[k].fileidx == i && g_symtab[k].index == rel.sym) // TODO: text section check
                {
                    sym = g_symtab[k];
                    break;
                }
            }

            // Undefined symbol (resolve)
            if (sym.flags & S_UNDF)
            {
                int found = 0;
                for (unsigned int k = 0; k < g_symcnt; k++)
                {
                    if (!strcmp(sym.name, g_symtab[k].name) && !(g_symtab[k].flags & S_UNDF))
                    {
                        found = 1;
                        sym = g_symtab[k];
                        break;
                    }
                }

                if (!found)
                    printf("Undefined symbol '%s'\n", sym.name); // TODO: error
            }

            struct sect sectsrc = g_sects[sym.fileidx]; //TODO: section index instead
            struct sect sectdst = g_sects[i];

            // Write the relocation: add file base, section base, and addend
            uint64_t val = s_base + sectsrc.base + sym.val + rel.addend;
            
            fseek(g_out, sectdst.base + rel.addr, SEEK_SET);
            fwrite(&val, rel.size, 1, g_out);
        }
    }
}

int main(int argc, char **argv)
{
    char *input[16] = { NULL };
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
                    s_base = strtoull(arg, NULL, 16);
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

    g_out = fopen("a.out", "w+");

    if (s_bin)
        do_binary();
    else
        do_normal();

    for (unsigned int i = 0; i < g_incnt; i++)
        fclose(g_in[i]);

    fclose(g_out);
    return 0;
}