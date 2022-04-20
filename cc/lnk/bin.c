// Binary file output
#include "lnk.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void do_binary()
{
    // 1st pass: Extract symbols from each input file (don't resolve globs, undfs yet)
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

    // 2nd pass: Write raw sections
    for (unsigned int s = SE_TEXT; s <= SE_BSS; s++)
    for (unsigned int f = 0; f < g_incnt; f++)
    {
        write_section(f, s);
    }

    // 3rd pass: Do text relocations and resolve S_UNDF symbols
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
                sym = resolve(&sym);

            // TODO: do this
            if (rel.flags & R_PCREL)
                continue;

            struct sect sectsrc = g_sects[sym.fileidx]; //TODO: section index instead
            struct sect sectdst = g_sects[i];

            // Write the relocation: add file base, section base, and addend
            uint64_t val = g_base + sectsrc.base + sym.val + rel.addend;

            fseek(g_out, sectdst.base + rel.addr, SEEK_SET);
            fwrite(&val, rel.size, 1, g_out);
        }
    }
}