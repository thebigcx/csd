#include <stdio.h>
#include <string.h>
#include <errno.h>

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("usage: cc <input>\n");
        return -1;
    }

    FILE *in = fopen(argv[1], "r");
    if (!in)
    {
        printf("%s: %s\n", argv[1], strerror(errno));
        return -1;
    }

    char *oname = strdup(argv[1]);
    oname[strlen(oname) - 1] = 's';

    FILE *out = fopen(oname, "w+");



    fclose(in);
    fclose(out);
    return 0;
}