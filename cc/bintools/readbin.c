#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include <lnk/bin.h>

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("usage: readbin <binary>\n");
        return -1;
    }

    FILE *f = fopen(argv[1], "r");
    if (!f)
    {
        printf("readbin: %s: %s\n", argv[1], strerror(errno));
        return -1;
    }

    struct bin_main main;
    fread(&main, sizeof(struct bin_main), 1, f);

    fseek(f, 0, SEEK_END);
    size_t len = ftell(f);
    fseek(f, main.strtab, SEEK_SET);

    char *strtab = malloc(len - main.strtab);
    fread(strtab, len - main.strtab, 1, f);

    printf("Binary File %s:\n", argv[1]);
    printf("\tEntry point: 0x%lx\n", main.entry);
    printf("\tText offset: 0x%lx\n", sizeof(struct bin_main));
    printf("\tText size: 0x%x\n", main.txtsz);
    printf("\tData offset: 0x%lx\n", sizeof(struct bin_main) + main.txtsz);
    printf("\tData size: 0x%x\n", main.datsz);

    printf("Symbols:\n");

    fseek(f, main.symtab, SEEK_SET);
    
    struct symbol s;
    for (unsigned int i = 0; i < main.strtab - main.symtab; i += sizeof(struct symbol))
    {
        fread(&s, sizeof(struct symbol), 1, f);
        printf("\t%s = 0x%lx\n", strtab + s.name, s.value);
    }

    fclose(f);
    return 0;
}