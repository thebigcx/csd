#include "asm.h"

#include <lnk/bin.h>
#include <string.h>

static struct bin_main s_main = { 0 };
static struct sect s_sects[3] = { 0 };
static int s_sectidx = -1; // Current section index

void binheader()
{
    fwrite(&s_main, sizeof(struct bin_main), 1, g_out);
}

void endsect()
{
    // Set size of current section (BSS will already have size)
    if (s_sectidx != -1 && s_sectidx != S_BSS)
    {
        struct sect *pre = &s_sects[s_sectidx];
        pre->size = ftell(g_out) - pre->offset;
    }
}

void setsect(int type)
{
    s_sectidx = type;
}

// TODO: check it goes text, data, bss
void startsect(char *name)
{
    endsect();

    int type;
    if (!strcmp(name, "text"))      type = SE_TEXT;
    else if (!strcmp(name, "data")) type = SE_DATA;
    else if (!strcmp(name, "bss"))  type = SE_BSS;
    else
        error("Invalid section name '%s'\n", name);

    s_sects[type] = (struct sect) {
        .offset = ftell(g_out),
        .type   = type
    };
    s_sectidx = type;
}

void binfini()
{
    endsect();

    // Write the main header
    fseek(g_out, 0, SEEK_END);

    s_main.txtsz = s_sects[SE_TEXT].size;
    s_main.datsz = s_sects[SE_DATA].size;
    s_main.bss   = g_bss;

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
            .name  = l->idx,
            .value = l->val,
            .flags = l->flags
        };

        fwrite(&s, sizeof(struct symbol), 1, g_out);
    }
}

struct sect *getsect()
{
    return &s_sects[s_sectidx];
}