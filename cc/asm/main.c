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

    /*struct code code = {
        .mnem = "add",
        .op[0] = {
            .mem = {
                .base = R_AX,
                .idx  = R_BX,
                .scale = 2,
                .disp = 70,
                .dispsz = 1,
                .size = 4
            },
            .type = OP_SB | OP_TM
        },
        .op[1] = {
            .imm = 69,
            .type = OP_SB | OP_TI
        }
    };*/
    struct code code = pscode();

    struct opcode opcode = matchop(&code);

    assem(&code, &opcode);

    fclose(g_in);
    fclose(g_out);
    return 0;
}