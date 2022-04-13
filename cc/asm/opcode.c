// Opcode table parser and matcher
#include "asm.h"

#include <string.h>
#include <stdlib.h>

static struct opcode *s_ops = NULL;
static unsigned int s_opcnt = 0;

void parse_opcodes()
{
    FILE *f = fopen("optbl.txt", "r");

    char *line = NULL;
    size_t n;
    
    // Current mnemonic
    char *mnem = NULL, *tok = NULL; 

    while (getline(&line, &n, f) != -1)
    {
        tok = strsep(&line, " ");

        if (!strcmp(tok, "mnem"))
        {
            // New mnemonic info
            tok = strsep(&line, " ");
            *strchr(tok, '\n') = 0;

            mnem = strdup(tok);
        }
        else
        {
            // Distinct opcode/instruction
            struct opcode op = { .mnem = mnem };
            
            tok += 2; // '0x'
            op.po = strtol(tok, NULL, 16);

            uint32_t *oper = &op.op1; // Current operand
            while ((tok = strsep(&line, " ")))
            {
                if (*tok == '/')
                {
                    // ModR/M.reg field
                    op.r = *(++tok) == 'r'
                         ? OR_REGR : *tok - '0';
                }
                else if (!strcmp(tok, "+R"))
                {
                    // REX.W prefix
                    op.rexw = 1;
                }
                else if (*tok == 'R')
                {
                    // r/m or reg operand
                    uint32_t type = OP_TR;

                    if (*(++tok) == 'M')
                    {
                        type |= OP_TM;
                        tok++;
                    }

                    size_t s = strtol(tok, NULL, 10) / 8; // Size in bytes

                    *oper++ = type | (s << 3);
                }
            }

            s_ops = realloc(s_ops, sizeof(struct opcode) * (s_opcnt + 1));
            s_ops[s_opcnt++] = op;
        }
    }

    fclose(f);
}

// Matches an opcode spec to code. It will be modified to contain
// relevant information for assembling (e.g. ModR/M.reg byte, etc.)
struct opcode matchop(struct code *code)
{
    static int init = 0;
    if (!init)
    {
        parse_opcodes();
        init = 1;
    }

    for (unsigned int i = 0; i < s_opcnt; i++)
    {
        struct opcode opc = s_ops[i]; // Copy it for modification

        if (strcmp(opc.mnem, code->mnem))
            goto next;

        // Match the operands
        uint32_t *op;
        struct op *cop;
        for (op = &opc.op1, cop = &code->op1; *op && cop->type; op++, cop++)
        {
            // Size or type doesn't match
            if (!(OP_TYPE(cop->type) & OP_TYPE(*op))
                || OP_SIZE(cop->type) != OP_SIZE(*op))
                goto next;

            if (opc.r == OR_REGR && OP_TYPE(*op) == OP_TR)
                opc.r = cop->reg;

            // TODO: specific register in operand check
        }

        return opc;

    next:
        continue;
    }

    // Doesn't exist
    printf("No such instruction.\n");
}