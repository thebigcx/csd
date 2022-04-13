// Main instruction assembler
#include "asm.h"

struct modrm
{
    uint8_t mod, reg, rm;
};

static void emitb(uint8_t b)
{
    fwrite(&b, 1, 1, g_out);
}

static void emit(uint64_t v)
{
         if (v < UINT8_MAX)  emitb(v);
    /*else if (v < UINT16_MAX) emitw(v);
    else if (v < UINT32_MAX) emitd(v);
    else if (v < UINT64_MAX) emitq(v);*/
}

#define REX_BASE (0b0100 << 4)

void emitrex(int w, struct modrm modrm)
{
    uint8_t rex = REX_BASE | (w << 3);

    rex |= (modrm.reg & 0b1000) << 2;
    // SIB: rex |= (modrm.reg & 0b1000) << 2;
    rex |= (modrm.rm & 0b1000) << 0;

    if (rex != REX_BASE)
        emit(rex);
}

void assem(struct code *code, struct opcode *opcode)
{
    uint8_t rm = OP_TYPE(opcode->op1) == (OP_TR | OP_TM) ? code->op1.reg
               : OP_TYPE(opcode->op2) == (OP_TR | OP_TM) ? code->op2.reg
               : code->op3.reg;

    struct modrm modrm = {
        .mod = 3,
        .reg = opcode->r,
        .rm  = rm
    };

    // REX
    emitrex(opcode->rexw, modrm);

    // Opcode
    if (opcode->of)
        emit(0x0f);

    emit(opcode->po);

    // ModR/M
    emit((modrm.mod << 6) | (modrm.reg << 3) | modrm.rm);
}