// Intermediate code representation
#include "asm.h"

#include <string.h>
#include <ctype.h>
#include <stdlib.h>

struct regstr
{
    const char *str;
    uint32_t val;
};

#define R_ENT(s, reg, size) { .str = s, .val = (reg << 7) | (size << 3) }

// TODO: put into optbl.txt file
static struct regstr s_regstrs[] = {
    R_ENT("al", R_AX, 1),
    R_ENT("cl", R_CX, 1),
    R_ENT("dl", R_DX, 1),
    R_ENT("bl", R_BX, 1),
    R_ENT("spl", R_SP, 1),
    R_ENT("bpl", R_BP, 1),
    R_ENT("sil", R_SI, 1),
    R_ENT("dil", R_DI, 1),

    R_ENT("ax", R_AX, 2),
    R_ENT("cx", R_CX, 2),
    R_ENT("dx", R_DX, 2),
    R_ENT("bx", R_BX, 2),
    R_ENT("sp", R_SP, 2),
    R_ENT("bp", R_BP, 2),
    R_ENT("si", R_SI, 2),
    R_ENT("di", R_DI, 2),

    R_ENT("eax", R_AX, 4),
    R_ENT("ecx", R_CX, 4),
    R_ENT("edx", R_DX, 4),
    R_ENT("ebx", R_BX, 4),
    R_ENT("esp", R_SP, 4),
    R_ENT("ebp", R_BP, 4),
    R_ENT("esi", R_SI, 4),
    R_ENT("edi", R_DI, 4),
    
    R_ENT("rax", R_AX, 8),
    R_ENT("rcx", R_CX, 8),
    R_ENT("rdx", R_DX, 8),
    R_ENT("rbx", R_BX, 8),
    R_ENT("rsp", R_SP, 8),
    R_ENT("rbp", R_BP, 8),
    R_ENT("rsi", R_SI, 8),
    R_ENT("rdi", R_DI, 8),
};

#define REGCNT (sizeof(s_regstrs) / sizeof(s_regstrs[0]))

// Parse register
uint32_t psreg(const char *str)
{
    // Search table for match
    for (unsigned int i = 0; i < REGCNT; i++)
    {
        if (!strcmp(str, s_regstrs[i].str)) return s_regstrs[i].val;        
    }

    return 0;
}

struct mem psmem()
{
    struct mem mem = {
        .base = R_NUL,
        .idx  = R_NUL,
        .size = 8
    };

    scan();

    while (g_tok.type != T_RBRACK)
    {
        // Parse (index * scale)
        if (g_tok.type == T_LPAREN)
        {
            scan(); // index
            mem.idx = OP_REG(psreg(g_tok.sv));
            scan(); // *
            scan(); // scale

            uint64_t scale = g_tok.iv;
            while (scale >>= 1) mem.scale++;

            scan(); // )
        }
        else if (g_tok.type == T_ILIT)
        {
            mem.disp = g_tok.iv;
            mem.dispsz = g_tok.iv < UINT8_MAX ? 1 : 4;
        }
        else if (g_tok.type == T_IDENT)
        {
            mem.base = OP_REG(psreg(g_tok.sv));
        }

        scan();
    }

    /*if (g_tok.type == T_ILIT)
    {
        mem.disp = g_tok.iv;
        mem.dispsz = g_tok.iv < UINT8_MAX ? 1 : 4;
        return mem;
    }

    mem.base = OP_REG(psreg(g_tok.sv));*/

    return mem;
}

struct op psop()
{
    // Immediate
    if (g_tok.type == T_ILIT)
    {
        int size = g_tok.iv < UINT8_MAX ? 1
                 : g_tok.iv < UINT16_MAX ? 2
                 : g_tok.iv < UINT32_MAX ? 4 : 8;

        return (struct op) {
            .type = OP_TI | (size << 3),
            .imm  = g_tok.iv
        };
    }
    else if (g_tok.type == T_LBRACK)
    {
        return (struct op) {
            .type = OP_TM | OP_SW, // TODO: determine size
            .mem  = psmem()
        };
    }

    // Try register
    uint32_t reg = psreg(g_tok.sv);
    if (reg)
    {
        return (struct op) {
            .type = OP_TR | OP_SIZE(reg),
            .reg  = OP_REG(reg)
        };
    }
}

struct code pscode()
{
    struct code code = { 0 };

    scan();

    code.mnem = strdup(g_tok.sv);

    // Parse operands
    scan();

    struct op *op = &code.op[0];
    while (1)
    {
        *op++ = psop();

        scan();

        if (g_tok.type == T_NL)
            break;
        else
            scan(); // Expect comma
    }

    return code;
}