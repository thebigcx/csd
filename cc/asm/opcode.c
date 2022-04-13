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
                    op.rex = 0b01001000;
                }
                else if (!strncmp(tok, "imm", 3))
                {
                    // Immediate operand
                    size_t s = strtol(tok += 3, NULL, 10) / 8;
                    *oper++ = OP_TI | (s << 3);
                }
                else if (*tok == 'i')
                {
                    // Immediate size
                    switch (*(++tok))
                    {
                        case 'b': op.imm = 1; break;
                        case 'w': op.imm = 2; break;
                        case 'd': op.imm = 4; break;
                        case 'q': op.imm = 8; break;
                    }
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
                else if (*tok == '%')
                {
                    *oper++ = psreg(++tok);
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
            // Specific register
            if (!OP_TYPE(*op))
            {
                if (OP_REG(*op) != cop->reg) goto next;
                else continue;
            }

            // Size or type doesn't match
            if (!(OP_TYPE(cop->type) & OP_TYPE(*op))
                || OP_SIZE(cop->type) != OP_SIZE(*op))
                goto next;

            if (opc.r == OR_REGR && OP_TYPE(*op) == OP_TR)
                opc.r = cop->reg;

            if (OP_TYPE(*op) == OP_TI)
                code->imm = cop->imm;

            if (OP_TYPE(cop->type) == OP_TM)
                code->mem = cop;

            // spl, bpl, sil, dil registers (require REX.W prefix for use)
            if (cop->reg >= R_SP && cop->reg <= R_DI && OP_SIZE(cop->type) == 8)
                opc.rex = 0b01000000; // TODO: ah, bh, ch, dh register macros (somehow?)
        }

        return opc;

    next:
        continue;
    }

    // Doesn't exist
    printf("No such instruction.\n");
}