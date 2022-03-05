#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    FILE *in = fopen(argv[1], "r");

    char *oname = strdup(argv[1]);
    oname[strlen(oname) - 1] = 's';

    FILE *out = fopen(oname, "w+");



    fclose(in);
    fclose(out);
    return 0;
}