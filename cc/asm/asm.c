// Main instruction assembler
#include "asm.h"

#include <stdlib.h>
#include <string.h>

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

unsigned int getpc()
{
    return ftell(g_out) - g_sect;
}

static struct label *s_lbls = NULL;
static unsigned int s_lblcnt = 0;

// Add a label to the list (or define an undefined one)
void addlabel(char *name, uint64_t pc)
{
    for (unsigned int i = 0; i < s_lblcnt; i++)
    {
        if (!strcmp(name, s_lbls[i].name))
        {
            s_lbls[i].val   = pc;
            return;
        }
    }

    s_lbls = realloc(s_lbls, sizeof(struct label) * (s_lblcnt + 1));
    s_lbls[s_lblcnt++] = (struct label) {
        .name = name,
        .val  = pc
    };
}

// Forward reference to resolve later
struct forward
{
    char *lbl;
    uint64_t pc;
    int size;
};

static struct forward *s_forwards = NULL;
static unsigned int s_forwardcnt = 0;

// Forward reference a label (add it as undefined, define it later)
void forwardref(char *name, int size)
{
    s_forwards = realloc(s_forwards, sizeof(struct forward) * (s_forwardcnt + 1));
    s_forwards[s_forwardcnt++] = (struct forward) {
        .lbl = name,
        .pc  = getpc(),
        .size = size
    };
}

struct label *resolvelbl(char *name)
{
    for (unsigned int i = 0; i < s_lblcnt; i++)
    {
        if (!strcmp(name, s_lbls[i].name))
            return &s_lbls[i];
    }
    return NULL;
}

// Resolve all forward references
void resolve_forwardrefs()
{
    for (unsigned int i = 0; i < s_forwardcnt; i++)
    {
        struct label *lbl = resolvelbl(s_forwards[i].lbl);
        if (!lbl)
            printf("Undefined label '%s'\n", s_forwards[i].lbl);

        fseek(g_out, g_sect + s_forwards[i].pc, SEEK_SET);
        emitv(lbl->val, s_forwards[i].size);
    }
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
    {
        // Use of label as displacement
        if (code->mem->lbl)
            forwardref(code->mem->lbl, code->mem->mem.dispsz);
        
        emitv(code->mem->mem.disp, code->mem->mem.dispsz);
    }

    // Immediate
    if (code->imm)
    {
        // Need to resolve label later
        if (code->imm->lbl)
            forwardref(code->imm->lbl, opcode->imm);
        
        emitv(code->imm->imm, opcode->imm);
    }
}