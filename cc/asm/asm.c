// Main instruction assembler
#include "asm.h"

#include <stdlib.h>
#include <string.h>

#include <lnk/bin.h>

struct modrm
{
    uint8_t mod, reg, rm;
    int used;
};

// Emit a byte (or extend BSS section)
void emitb(uint8_t b)
{
    if (getsect()->type == SE_BSS)
        g_bss++;
    else
        fwrite(&b, 1, 1, g_out);
}

void emitw(uint16_t w)
{
    emitb(w &  0xff);
    emitb(w >> 8);
}

void emitd(uint32_t d)
{
    emitw(d &  0xffff);
    emitw(d >> 16);
}

void emitq(uint64_t q)
{
    emitd(q &  0xffffffff);
    emitd(q >> 32);
}

void emitv(uint64_t v, int size)
{
    switch (size)
    {
        case 1: emitb(v); break;
        case 2: emitw(v); break;
        case 4: emitd(v); break;
        case 8: emitq(v); break;
    }
}

void emit(uint64_t v)
{
         if (v < UINT8_MAX)  emitb(v);
    else if (v < UINT16_MAX) emitw(v);
    else if (v < UINT32_MAX) emitd(v);
    else if (v < UINT64_MAX) emitq(v);
}

unsigned int getpc()
{
    return ftell(g_out) - getsect()->offset;
}

static struct label *s_lbls = NULL;
static unsigned int s_lblcnt = 0;

static void addlabel_impl(struct label l)
{
    s_lbls = realloc(s_lbls, sizeof(struct label) * (s_lblcnt + 1));
    s_lbls[s_lblcnt++] = l;
}

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

    int sect = getsect()->type;
    uint32_t flags = sect == SE_TEXT ? S_TEXT
                   : sect == SE_DATA ? S_DATA
                   : sect == SE_BSS  ? S_BSS : 0;

    addlabel_impl((struct label) {
        .name  = name,
        .val   = pc,
        .flags = flags
    });
}

void addextern(char *name)
{
    addlabel_impl((struct label) {
        .name  = name,
        .flags = S_UNDF
    });
}

// Forward reference a label (add it as undefined, define it later)
void forwardref(char *name, int size, int pcrel)
{
    g_forwards = realloc(g_forwards, sizeof(struct forward) * (g_forwardcnt + 1));
    g_forwards[g_forwardcnt++] = (struct forward) {
        .lbl   = name,
        .pc    = getpc(),
        .sect  = getsect(),
        .size  = size,
        .line  = g_line,
        .flags = pcrel ? R_PCREL : 0
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

struct label *getlbls()
{
    return s_lbls;
}

// Get number of labels
unsigned int labelcnt()
{
    return s_lblcnt;
}

// TODO: is this necessary?
// Resolve all forward references
void resolve_forwardrefs()
{
    for (unsigned int i = 0; i < g_forwardcnt; i++)
    {
        // Set line number to the forward reference
        g_line = g_forwards[i].line;

        struct label *lbl = resolvelbl(g_forwards[i].lbl);
        if (!lbl)
            error("Undefined label '%s'\n", g_forwards[i].lbl);

        /*fseek(g_out, g_forwards[i].sect->offset + g_forwards[i].pc, SEEK_SET);
        
        if (g_forwards[i].flags & R_PCREL)
            emitv(lbl->val - (g_forwards[i].pc + g_forwards[i].size), g_forwards[i].size);
        else
            emitv(lbl->val, g_forwards[i].size);*/
    }
}

#define REX_BASE (0b0100 << 4)

void emitrex(struct opcode *opcode, struct modrm modrm)
{
    uint8_t rex = opcode->rex;

    // Instructions with +r use REX.B to encode highest bit
    if (opcode->r & OR_REGP)
        modrm.rm = opcode->r & 0b1111;

    rex |= !!(modrm.reg & 0b1000) << 2;
    // SIB: rex |= (modrm.reg & 0b1000) << 2;
    rex |= !!(modrm.rm & 0b1000) << 0;

    if (rex)
        emit(REX_BASE | (rex & 0b1111));
}

// Create ModR/M and SIB bytes
void modrmsib(struct modrm *modrm, struct code *code, struct opcode *opcode)
{
    if (!code->mem && !code->rm && (opcode->r == OR_UNUSED || opcode->r & OR_REGP))
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
    unsigned int pc = getpc();

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
        {
            if (!strcmp(code->mem->lbl, "."))
                code->mem->mem.disp = pc;
            else
                forwardref(strdup(code->mem->lbl), code->mem->mem.dispsz, 0);
        }

        emitv(code->mem->mem.disp, code->mem->mem.dispsz);
    }

    // RIP-relative: subtract address of following instruction
    if (opcode->rel)
        code->imm->imm -= getpc() + opcode->imm;

    // Immediate
    if (code->imm)
    {
        // Need to resolve label later
        if (code->imm->lbl)
        {
            if (!strcmp(code->imm->lbl, "."))
                code->imm->imm += pc; // In case of RIP-rel
            else
                forwardref(strdup(code->imm->lbl), opcode->imm, opcode->rel);
        }
        
        emitv(code->imm->imm, opcode->imm);
    }
}