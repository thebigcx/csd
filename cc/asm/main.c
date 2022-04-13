#include "asm.h"

#include <errno.h>
#include <string.h>

FILE *g_in  = NULL;
FILE *g_out = NULL;
struct tok g_tok = { 0 };

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

    struct code code = {
        .mnem = "add",
        .op1 = {
            .mem = {
                .base = R_AX,
                .idx  = R_NUL,
                .disp = 70,
                .dispsz = 1
            },
            .type = OP_SB | OP_TM
        },
        .op2 = {
            .imm = 69,
            .type = OP_SB | OP_TI
        }
    };

    struct opcode opcode = matchop(&code);

    assem(&code, &opcode);

    fclose(g_in);
    fclose(g_out);
    return 0;
}