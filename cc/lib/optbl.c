#include "optbl.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// TODO: TEMPORARY!! Need to actually parse the register
static op_t parse_reg(const char *str)
{
    return (op_t) { .type = OTT_REG, .reg = 0, .size = 2 };
}

static struct optbl parse_line(char *str, const char *mnem)
{
    const char *tok = str;

    struct optbl op = { .flag = OT_NOMODRM };

    strcpy(op.mnem, mnem);
    
    op.po = strtol(tok + 2, NULL, 16);

    op_t *oper = &op.ops[0]; // Current operand
    while ((tok = strsep(&str, " ")))
    {
        if (*tok == '/') { // ModR/M.reg field
            op.flag &= ~OT_NOMODRM;

            if (*(++tok) == 'r') op.flag |= OT_REGR;
            else op.modr = *tok - '0';
        } else if (!strcmp(tok, "+r")) // Add reg to primary opcode
            op.flag |= OT_REGPO;
        else if (!strcmp(tok, "+R")) // REX.W prefix
            op.flag |= OT_REXW;
        else if (!strncmp(tok, "imm", 3)) {
            // Immediate operand
            size_t size = strtol(tok += 3, NULL, 10) / 8;
            *oper++ = (op_t) {
                .type = OTT_IMM,
                .size = size
            };
        } else if (!strncmp(tok, "rel", 3)) {
            // Relative address operand
            size_t size = strtol(tok += 3, NULL, 10) / 8;
            *oper++ = (op_t) {
                .type = OTT_IMM,
                .size = size
            };
            op.flag |= OT_REL;
        } else if (*tok == 'R' || *tok == 'M') {
            // r, m, or r/m operand
            uint32_t type = 0;
            if (*tok == 'R') {
                tok++;
                type = OTT_REG;
            }
            if (*tok == 'M') {
                tok++;
                type |= OTT_MEM;
            }

            size_t size = strtol(tok, NULL, 10) / 8; // Size in bytes

            *oper++ = (op_t) {
                .type = type,
                .size = size
            };
        } else if (*tok == '%')
            *oper++ = parse_reg(++tok);
        //else
            //general_error("Malformed ~/opt/share/optbl.txt file. Please reinstall assembler or redownload file.\n");
    }

    return op;
}

int optbl_foreach(const char *file, void (*fn)(struct optbl*))
{
    FILE *f = fopen(file, "r");
    if (!f) return -1;

    char *line = NULL;
    size_t n = 0;
    
    // Current mnemonic
    char *mnem = NULL;

    while (getline(&line, &n, f) != -1) {
        if (*line == '\n') continue;

        if (!strncmp(line, "mnem", 4)) {
            // New mnemonic info
            free(mnem);
            
            char *tok = line + 5;
            mnem = strndup(tok, strlen(tok) - 1);
        } else {
            // Distinct opcode/instruction
            struct optbl op = parse_line(line, mnem);
            fn(&op);
        }
    }

    free(line);
    fclose(f);

    return 0;
}

// TODO: this is so stupid
static struct optbl *s_curr_op = NULL; /* Current optbl pointer */
static uint8_t s_opcode = 0;
static uint8_t s_pre = 0;
static uint8_t s_size = 0;
static int s_done = 0;

static void search_callback(struct optbl *op)
{
    if (s_done) return;

    if (op->po == s_opcode && (!!(op->flag & OT_INPRE) == (s_pre == 0x0f))) {
        
        // TODO: this is kinda bad
        // 8-bit instructions have designated opcodes
        if (op->ops[0].size != 1) {
            if (op->ops[0].type & (OTT_REG | OTT_MEM) && op->ops[0].size != s_size) return;
            if (op->ops[1].type & (OTT_REG | OTT_MEM) && op->ops[1].size != s_size) return;
        }

        *s_curr_op = *op;
        s_done = 1;
    }
}

int optbl_from_opcode(const char *file, uint8_t pre, uint8_t opcode, uint8_t size,
                      struct optbl *op)
{
    s_opcode  = opcode;
    s_pre     = pre;
    s_curr_op = op;
    s_size    = size;
    s_done    = 0;

    return optbl_foreach(file, search_callback);
}