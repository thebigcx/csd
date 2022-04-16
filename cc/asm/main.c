#include "asm.h"

#include <errno.h>
#include <string.h>

FILE *g_in  = NULL;
FILE *g_out = NULL;
struct tok g_tok = { 0 };
int g_mode = 8;
struct forward *g_forwards = NULL;
unsigned int g_forwardcnt = 0;

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("usage: asm <input>\n");
        return -1;
    }

    g_in = fopen(argv[1], "r");
    if (!g_in)
    {
        printf("%s: %s\n", argv[1], strerror(errno));
        return -1;
    }

    char *oname = strdup(argv[1]);
    oname[strlen(oname) - 1] = 'o';

    g_out = fopen(oname, "w+");

    /*struct code code = pscode();

    struct opcode opcode = matchop(&code);

    assem(&code, &opcode);*/
    dofile();

    fclose(g_in);
    fclose(g_out);
    return 0;
}