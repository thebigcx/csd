#include "cc.h"

const char *regs[] = { "rax", "rbx", "rcx", "rdx" };

#define REGCNT (sizeof(regs) / sizeof(regs[0]))
#define NOREG (-1)

static int s_reglst[REGCNT] = { 0 };

// Allocate a general purpose register
int ralloc()
{
    int i;
    for (i = 0; i < (int)REGCNT && s_reglst[i]; i++);
    s_reglst[i] = 1;
    return i;
}

void rfree(int r)
{
    s_reglst[r] = 0;
}

// Discard the result of an operation (register)
void discard(int r)
{
    if (r != NOREG) rfree(r);
}

int cgilit(struct ast *ast)
{
    int r = ralloc();
    fprintf(g_out, "\tmov %s, %d\n", regs[r], ast->iv);
    return r;
}

int cgadd(int r1, int r2)
{
    fprintf(g_out, "\tadd %s, %s\n", regs[r1], regs[r2]);
    rfree(r2);
    return r1;
}

int cgbinop(struct ast *ast)
{
    int r1 = cg(ast->left);
    int r2 = cg(ast->right);
    return cgadd(r1, r2);
}

int cg(struct ast *ast)
{
    switch (ast->type)
    {
        case A_BINOP: return cgbinop(ast);
        case A_ILIT:  return cgilit(ast);
    }
}