#include "asm.h"

#include <lnk/bin.h>
#include <string.h>

static struct bin_main s_main = { 0 };
static uint64_t s_sectoff = 0;
static uint64_t s_datoff = 0;

void binheader()
{
    fwrite(&s_main, sizeof(struct bin_main), 1, g_out);
    setsect("text");
}

void setsect(char *name)
{
    if (!strcmp(name, "text"))
        s_sectoff = sizeof(struct bin_main);
    else if (!strcmp(name, "data"))
    {
        if (s_datoff)
            s_sectoff = s_datoff;
        else
            s_sectoff = s_datoff = ftell(g_out);
    }
}

void binfini()
{
    // Write the main header
    fseek(g_out, 0, SEEK_END);

    s_main.txtsz = s_datoff - sizeof(struct bin_main);
    s_main.datsz = ftell(g_out) - s_datoff;

    s_main.txtrel = ftell(g_out);
    s_main.datrel = s_main.txtrel + sizeof(struct rel) * g_forwardcnt;

    s_main.symtab = s_main.datrel; // TODO: number of relocs
    s_main.strtab = s_main.symtab + labelcnt() * sizeof(struct symbol);

    fseek(g_out, 0, SEEK_SET);
    fwrite(&s_main, sizeof(struct bin_main), 1, g_out);

    fseek(g_out, s_main.txtrel, SEEK_SET);

    for (unsigned int i = 0; i < g_forwardcnt; i++)
    {
        char *lbl = g_forwards[i].lbl;

        // Find symbol
        unsigned int sym;
        for (sym = 0; sym < labelcnt(); sym++)
            if (!strcmp(lbl, getlbls()[sym].name)) break;

        // Write relocation
        struct rel r = {
            .addr  = g_forwards[i].pc,
            .size  = g_forwards[i].size,
            .sym   = sym,
            .flags = g_forwards[i].flags
        };
        fwrite(&r, sizeof(struct rel), 1, g_out);
    }

    fseek(g_out, s_main.symtab, SEEK_SET);
    
    for (unsigned int i = 0; i < s_main.strtab - s_main.symtab; i++)
        fputc(0, g_out);

    // Write the strings
    fputc(0, g_out);
    for (unsigned int i = 0; i < labelcnt(); i++)
    {
        struct label *l = &getlbls()[i];
        l->idx = ftell(g_out) - s_main.strtab;

        fputs(l->name, g_out);
        fputc(0, g_out);
    }

    // Write the symbols
    fseek(g_out, s_main.symtab, SEEK_SET);

    for (unsigned int i = 0; i < labelcnt(); i++)
    {
        struct label *l = &getlbls()[i];

        struct symbol s = {
            .name = l->idx,
            .value = l->val,
            .flags = l->flags
        };

        fwrite(&s, sizeof(struct symbol), 1, g_out);
    }
}

uint64_t getsect()
{
    return s_sectoff;
}

char *getsectname()
{
    if (s_sectoff == sizeof(struct bin_main)) return "text";
    else return "data";
}