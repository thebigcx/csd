#include <stdio.h>
#include <string.h>
#include <errno.h>

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

    printf("Binary File %s:\n", argv[1]);
    printf("\tEntry point: 0x%lx\n", main.entry);
    printf("\tText offset: 0x%lx\n", sizeof(struct bin_main));
    printf("\tText size: 0x%x\n", main.txtsz);
    printf("\tData offset: 0x%lx\n", sizeof(struct bin_main) + main.txtsz);
    printf("\tData size: 0x%x\n", main.datsz);

    fclose(f);
    return 0;
}