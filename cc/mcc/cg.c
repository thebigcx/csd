#include "mcc.h"

#include <string.h>

#define NREG (-1)

#define RAX 0
#define RBX 1
#define RCX 2
#define RDX 3
#define R8  4
#define R9  5
#define R10 6
#define R11 7
#define R12 8
#define R13 9
#define R14 10
#define R15 11

#define RCNT 12

static FILE *s_out = NULL;

static const char *s_r64[] = {
    [RAX] = "rax",
    [RBX] = "rbx",
    [RCX] = "rcx",
    [RDX] = "rdx",
    [R8]  = "r8",
    [R9]  = "r9",
    [R10] = "r10",
    [R11] = "r11",
    [R12] = "r12",
    [R13] = "r13",
    [R14] = "r14",
    [R15] = "r15"
};

static int s_rmap[RCNT] = { 0 };

// Allocate register
int ralloc()
{
    int i = -1;
    while (s_rmap[++i]);

    s_rmap[i] = 1;
    return i;
}

// Free register
void rfree(int r)
{
    s_rmap[r] = 0;
}

int cgassign(struct ast *ast)
{
    int rhs = cg(ast->rhs);

    struct sym *sym = lookup(ast->lhs->val);
    fprintf(s_out, "\tmov [rsp - %d], %s\n", symstckoff(sym), s_r64[rhs]);

    return rhs;
}

// Generate binary operation
int cgbinop(struct ast *ast)
{
    if (!strcmp(ast->val, "=")) return cgassign(ast);

    int rhs = cg(ast->rhs);
    int lhs = cg(ast->lhs);

    const char *inst = NULL;

    if (!strcmp(ast->val, "+")) inst = "add";
    if (!strcmp(ast->val, "-")) inst = "sub";

    fprintf(s_out, "\t%s %s, %s\n", inst, s_r64[lhs], s_r64[rhs]);

    rfree(rhs);
    return lhs;
}

// Generate int literal
int cgilit(struct ast *ast)
{
    int r = ralloc();
    fprintf(s_out, "\tmov %s, %s\n", s_r64[r], ast->val);
    return r;
}

int cgid(struct ast *ast)
{
    int r = ralloc();

    struct sym *s = lookup(ast->val);
    fprintf(s_out, "\tmov %s, [rsp - %d] ; <%s>\n", s_r64[r], s->off, s->name);

    return r;
}

int cg(struct ast *ast)
{
    switch (ast->type) {
        case A_BINOP: return cgbinop(ast);
        case A_ILIT:  return cgilit(ast);
        case A_ID:    return cgid(ast);
    }

    return NREG;
}

void cgfile(FILE *out)
{
    s_out = out;
}

void cgfndef(char *name)
{
    fprintf(s_out, "%s:\n", name);
}

void cgfnend()
{
    fprintf(s_out, "\tret\n");
}