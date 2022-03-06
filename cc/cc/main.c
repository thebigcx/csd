#include "cc.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

FILE *g_in = NULL;
FILE *g_out = NULL;
struct tok g_tok = { 0 };

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("usage: cc <input>\n");
        return -1;
    }

    g_in = fopen(argv[1], "r");
    if (!g_in)
    {
        printf("%s: %s\n", argv[1], strerror(errno));
        return -1;
    }

    char *oname = strdup(argv[1]);
    oname[strlen(oname) - 1] = 's';

    g_out = fopen(oname, "w+");

    scan();
    struct ast *ast = cmpdstmt();
    cg(ast);

    fclose(g_in);
    fclose(g_out);
    return 0;
}