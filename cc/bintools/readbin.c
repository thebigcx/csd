// readbin: print the metadata about a binary file

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include <lnk/bin.h>

FILE *g_f = NULL;

static void cleanup()
{
    fclose(g_f);
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("usage: readbin <binary>\n");
        return -1;
    }

    if (!(g_f = fopen(argv[1], "r"))) {
        printf("readbin: %s: %s\n", argv[1], strerror(errno));
        return -1;
    }

    struct bin_main main;
    fread(&main, sizeof(struct bin_main), 1, g_f);

    fseek(g_f, 0, SEEK_END);
    size_t len = ftell(g_f);
    fseek(g_f, main.strtab, SEEK_SET);

    char *strtab = malloc(len - main.strtab);
    fread(strtab, len - main.strtab, 1, g_f);

    printf("Binary File %s:\n", argv[1]);
    printf("\tEntry point: 0x%lx\n", main.entry);
    printf("\tText offset: 0x%lx\n", sizeof(struct bin_main));
    printf("\tText size: 0x%x\n", main.txtsz);
    printf("\tData offset: 0x%lx\n", sizeof(struct bin_main) + main.txtsz);
    printf("\tData size: 0x%x\n", main.datsz);
    printf("\tBSS size: 0x%x\n", main.bss);

    printf("Symbols:\n");

    fseek(g_f, main.symtab, SEEK_SET);

    size_t symtabsz = main.strtab - main.symtab;
    struct symbol *syms = malloc(symtabsz);
    fread(syms, symtabsz, 1, g_f);

    for (unsigned int i = 0; i < symtabsz / sizeof(struct symbol); i++) {
        const char *sect = syms[i].flags & S_TEXT ? "text"
                         : syms[i].flags & S_DATA ? "data"
                         : syms[i].flags & S_BSS  ? "bss" : "<unknown section>";

        printf("\t%s: %s = 0x%lx", sect, strtab + syms[i].name, syms[i].value);

        if (syms[i].flags & S_UNDF) printf(" UNDF");
        if (syms[i].flags & S_GLOB) printf(" GLOB");

        printf("\n");
    }

    printf("Text relocations:\n");
    
    fseek(g_f, main.txtrel, SEEK_SET);

    struct rel r;
    for (unsigned int i = 0; i < main.datrel - main.txtrel; i += sizeof(struct rel)) {
        fread(&r, sizeof(struct rel), 1, g_f);

        printf("\tat text + 0x%lx: %s ", r.addr, strtab + syms[r.sym].name);
        if (r.addend < 0)
            printf("- %ld ", -r.addend);
        else
            printf("+ %ld ", r.addend);

        printf("(Size %d)\n", r.size);
    }

    printf("Data relocations:\n");

    for (unsigned int i = 0; i < main.symtab - main.datrel; i += sizeof(struct rel)) {
        fread(&r, sizeof(struct rel), 1, g_f);

        printf("\tat data + 0x%lx: %s ", r.addr, strtab + syms[r.sym].name);
        if (r.addend < 0)
            printf("- %ld ", -r.addend);
        else
            printf("+ %ld ", r.addend);

        printf("(Size %d)\n", r.size);
    }

    free(syms);
    free(strtab);
    return 0;
}