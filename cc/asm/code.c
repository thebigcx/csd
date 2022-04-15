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

struct mem psmem(struct op *op)
{
    struct mem mem = {
        .base = R_NUL,
        .idx  = R_NUL,
        .size = 8
    };

    expect(T_LBRACK);

    while (g_tok.type != T_RBRACK)
    {
        // Parse (index * scale)
        if (g_tok.type == T_LPAREN)
        {
            scan(); // index
            mem.idx = OP_REG(psreg(g_tok.sv));
            expect(T_IDENT);
            expect(T_STAR);

            uint64_t scale = g_tok.iv;
            while (scale >>= 1) mem.scale++;

            expect(T_ILIT);
        }
        else if (g_tok.type == T_ILIT)
        {
            mem.disp = g_tok.iv;
            mem.dispsz = g_tok.iv < UINT8_MAX ? 1 : 4;
        }
        else if (g_tok.type == T_IDENT)
        {
            uint32_t reg;
            if (!(reg = psreg(g_tok.sv)))
            {
                op->lbl = g_tok.sv;
                mem.dispsz = 4;
            }
            else
                mem.base = OP_REG(reg);
        }

        scan();
    }

    return mem;
}

uint32_t pssize(const char *str)
{
    if (*str++ != 'u') return 0;
    return strtol(str, NULL, 10) / 8;
}

struct op psop()
{
    // Size attribute (u8, u16, etc.)
    uint32_t opsz = pssize(g_tok.sv);
    if (opsz) scan();

    // Immediate
    if (g_tok.type == T_ILIT)
    {
        // TODO: immediates can be less than actual size
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
        if (!opsz)
        {
            // TODO: error
            printf("Must specify size for memory address.\n");
        }

        struct op op = {
            .type = OP_TM | (opsz << 3)
        };
        op.mem = psmem(&op);
        return op;
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
    
    return (struct op) {
        .type = OP_TI | OP_SD,
        .lbl  = g_tok.sv
    };
}

struct code pscode()
{
    struct code code = { 0 };

    code.mnem = strdup(g_tok.sv);
    expect(T_IDENT);

    // Parse operands
    
    struct op *op = &code.op[0];
    while (1)
    {
        *op++ = psop();

        scan();

        if (g_tok.type == T_NL)
            break;
        else
            expect(T_COMMA);
    }

    return code;
}

// Do assembler directive
int directive()
{
    if (!strcmp(g_tok.sv, "code"))
    {
        scan();
        g_mode = g_tok.iv / 8;
        expect(T_ILIT);
    }
    else if (!strcmp(g_tok.sv, "section"))
    {
        scan();
        setsect(g_tok.sv);
        expect(T_IDENT);
    }
    /*else if (!strcmp(g_tok.sv, "times"))
    {
        scan();

        expect(T_ILIT);
    }*/
    else if (*g_tok.sv == 'd') // Define byte
    {
        char s = *(g_tok.sv + 1);

        switch (s)
        {
            case 'b': scan(); emitb(g_tok.iv); break;
            case 'w': scan(); emitw(g_tok.iv); break;
            case 'd': scan(); emitd(g_tok.iv); break;
            case 'q': scan(); emitq(g_tok.iv); break;
            default: return 0;
        }
        expect(T_ILIT);
    }
    else if (!strcmp(g_tok.sv, "entry"))
    {
        scan();
        //setentry(g_tok.sv);
        expect(T_IDENT);
    }
    else return 0;

    return 1;
}

// Assemble file
void dofile()
{
    binheader();

    scan();
    while (1)
    {
        while (g_tok.type == T_NL) scan();
        if (g_tok.type == T_EOF) break;

        if (g_tok.type == T_COLON)
        {
            expect(T_COLON);
            addlabel(g_tok.sv, getpc());
            expect(T_IDENT);
        }
        else if (!directive())
        {
            struct code code = pscode();
            struct opcode op = matchop(&code);
            
            assem(&code, &op);
        }
    }

    resolve_forwardrefs();
    binfini();
}