// Main instruction assembler
#include "asm.h"

struct modrm
{
    uint8_t mod, reg, rm;
    int used;
};

static void emitb(uint8_t b)
{
    fwrite(&b, 1, 1, g_out);
}

static void emitw(uint16_t w)
{
    emitb(w &  0xff);
    emitb(w >> 8);
}

static void emitd(uint32_t d)
{
    emitw(d &  0xffff);
    emitw(d >> 16);
}

static void emitq(uint64_t q)
{
    emitd(q &  0xffffffff);
    emitd(q >> 32);
}

static void emitv(uint64_t v, int size)
{
    switch (size)
    {
        case 1: emitb(v); break;
        case 2: emitw(v); break;
        case 4: emitd(v); break;
        case 8: emitq(v); break;
    }
}

static void emit(uint64_t v)
{
         if (v < UINT8_MAX)  emitb(v);
    else if (v < UINT16_MAX) emitw(v);
    else if (v < UINT32_MAX) emitd(v);
    else if (v < UINT64_MAX) emitq(v);
}

#define REX_BASE (0b0100 << 4)

void emitrex(struct opcode *opcode, struct modrm modrm)
{
    uint8_t rex = opcode->rex;

    rex |= (modrm.reg & 0b1000) << 2;
    // SIB: rex |= (modrm.reg & 0b1000) << 2;
    rex |= (modrm.rm & 0b1000) << 0;

    if (rex)
        emit(rex);
}

// Create ModR/M and SIB bytes
void modrmsib(struct modrm *modrm, struct code *code, struct opcode *opcode)
{
    if (!code->mem && !code->rm && opcode->r == OR_UNUSED)
        return;

    modrm->used = 1;
    modrm->reg  = opcode->r;
    
    if (!code->mem)
    {
        modrm->mod = 3;
        modrm->rm  = code->rm->reg;
    }
    else
    {
        struct mem *mem = &code->mem->mem;

        if (mem->idx == R_NUL && mem->base != R_NUL
            && (mem->base != R_SP && mem->base != R_R12))
        {
            modrm->rm = mem->base;
            modrm->mod = mem->dispsz;

            if (modrm->mod == 4) modrm->mod = 2;
        }
        else
        {
            modrm->rm = R_SP;
            mem->used = 1;

            if (mem->dispsz == 1)
                modrm->mod = 1;
            else
                modrm->mod = 2;

            if (mem->idx == R_NUL)
                mem->idx = R_SP;
        }
    }
}

void assem(struct code *code, struct opcode *opcode)
{
    struct modrm modrm = { 0 };
    modrmsib(&modrm, code, opcode);    

    // Operand/address size overrides
    if (opcode->osovr)
        emit(opcode->osovr);
    if (opcode->asovr)
        emit(opcode->asovr);

    // REX
    emitrex(opcode, modrm);

    // Opcode
    if (opcode->of)
        emit(0x0f);

    emit(opcode->po);

    // ModR/M
    // TODO: only emit it when necessary
    if (modrm.used)
        emit((modrm.mod << 6) | (modrm.reg << 3) | modrm.rm);

    if (code->mem && code->mem->mem.used)
        emit((code->mem->mem.scale << 6) | (code->mem->mem.idx << 3) | code->mem->mem.base);

    // Displacement
    if (code->mem)
        emitv(code->mem->mem.disp, code->mem->mem.dispsz);

    // Immediate
    emitv(code->imm, opcode->imm);
}