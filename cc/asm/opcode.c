// Opcode table parser and matcher
#include "asm.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>

static struct opcode *s_ops = NULL;
static unsigned int s_opcnt = 0;

void parse_opcodes()
{
    // Construct path to optbl.txt
    char *home = getenv("HOME");
    const char *opt = "/opt/share/optbl.txt";
    char *path = calloc(1, strlen(home) + strlen(opt) + 1);

    strcat(path, home);
    strcat(path, opt);

    FILE *f = fopen(path, "r");
    if (!f)
        general_error("Could not open %s: %s\n", realpath(path, NULL), strerror(errno));

    free(path);

    char *line = NULL;
    size_t n;
    
    // Current mnemonic
    char *mnem = NULL, *tok = NULL; 

    while (getline(&line, &n, f) != -1)
    {
        if (*line == '\n') continue;

        char *strp = line;
        tok = strsep(&strp, " ");

        if (!strcmp(tok, "mnem"))
        {
            // New mnemonic info
            tok = strsep(&strp, " ");
            *strchr(tok, '\n') = 0;

            mnem = strdup(tok);
        }
        else
        {
            // Distinct opcode/instruction
            struct opcode op = { .mnem = mnem, .r = OR_UNUSED };
            
            tok += 2; // '0x'
            op.po = strtol(tok, NULL, 16);

            uint32_t *oper = &op.op[0]; // Current operand
            while ((tok = strsep(&strp, " ")))
            {
                if (*tok == '/')
                {
                    // ModR/M.reg field
                    op.r = *(++tok) == 'r'
                         ? OR_REGR : *tok - '0';
                }
                else if (!strcmp(tok, "+r"))
                {
                    // Add reg to primary opcode
                    op.r = OR_REGP;
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
                else if (!strncmp(tok, "rel", 3))
                {
                    // Relative address operand
                    size_t s = strtol(tok += 3, NULL, 10) / 8;
                    *oper++ = OP_TI | (s << 3);
                    op.rel  = s;
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
                else if (*tok == 'R' || *tok == 'M')
                {
                    // r, m, or r/m operand
                    uint32_t type = 0;
                    if (*tok == 'R')
                    {
                        tok++;
                        type = OP_TR;
                    }
                    if (*tok == 'M')
                    {
                        tok++;
                        type |= OP_TM;
                    }

                    size_t s = strtol(tok, NULL, 10) / 8; // Size in bytes

                    *oper++ = type | (s << 3);
                }
                else if (*tok == '%')
                {
                    *oper++ = psreg(++tok);
                }
                else
                    general_error("Malformed ~/opt/share/optbl.txt file. Please reinstall assembler or redownload file.\n");
            }

            s_ops = realloc(s_ops, sizeof(struct opcode) * (s_opcnt + 1));
            s_ops[s_opcnt++] = op;
        }

        free(line);
        line = NULL;
    }

    free(line);
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
        for (op = &opc.op[0], cop = &code->op[0]; *op && cop->type; op++, cop++)
        {
            // Check size (doesn't matter if immediate)
            if (OP_TYPE(cop->type) != OP_TI && OP_SIZE(cop->type) != OP_SIZE(*op))
                goto next;

            // Specific register
            if (!OP_TYPE(*op))
            {
                if (OP_REG(*op) != cop->reg) goto next;
                else continue;
            }

            // Size or type doesn't match
            if (!(OP_TYPE(cop->type) & OP_TYPE(*op)))
                goto next;

            // Set the size of immediate to whatever is required
            if (OP_TYPE(cop->type) == OP_TI)
            {
                cop->type &= ~OP_SMASK;
                cop->type |= OP_SIZE(*op);
            }

            if (g_mode == 8 && OP_SIZE(cop->type) == OP_SW
                || g_mode == 4 && OP_SIZE(cop->type) == OP_SW
                || g_mode == 2 && OP_SIZE(cop->type) == OP_SD)
                opc.osovr = 0x66;

            // ModR/M.reg field
            if (opc.r == OR_REGR && OP_TYPE(*op) == OP_TR)
                opc.r = cop->reg;

            // +r : add reg to primary opcode
            if (opc.r == OR_REGP && OP_TYPE(*op) == OP_TR)
            {
                opc.po += cop->reg & 0b111;
                opc.r |= cop->reg;
            }

            // Immediate
            if (OP_TYPE(*op) == OP_TI)
                code->imm = cop;

            // Memory address
            if (OP_TYPE(cop->type) == OP_TM)
            {
                code->mem = cop;
                opc.asovr = cop->mem.size == 4 ? 0x67 : 0;
            }

            // R/M field
            if (OP_TYPE(*op) == (OP_TR | OP_TM))
                code->rm = cop;

            // spl, bpl, sil, dil registers (require REX.W prefix for use)
            if (cop->reg >= R_SP && cop->reg <= R_DI && OP_SIZE(cop->type) == 8)
                opc.rex = 0b01000000; // TODO: ah, bh, ch, dh register macros (somehow?)
        }

        return opc;

    next:
        continue;
    }

    // Instruction is not defined
    error("No such instruction\n");
    __builtin_unreachable();
}