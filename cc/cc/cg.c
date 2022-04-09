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

int cgident(struct ast *ast)
{
    int r = ralloc();
    return r;
}

int cgadd(int r1, int r2)
{
    fprintf(g_out, "\tadd %s, %s\n", regs[r1], regs[r2]);
    rfree(r2);
    return r1;
}

int cgassign(struct ast *left, int r)
{
    fprintf(g_out, "\tmov %s, %s\n", left->sv, regs[r]);
    return r;
}

int cgbinop(struct ast *ast)
{
    int r1;
    if (ast->mid->op != OP_ASSIGN) // Assignments need not generate LHS
        r1 = cg(ast->left);
    
    int r2 = cg(ast->right);

    switch (ast->mid->op)
    {
        case OP_ADD: return cgadd(r1, r2);
        case OP_ASSIGN: return cgassign(ast->left, r2);
    }
}

// Generate code for compound statement
int cgcmpd(struct ast *ast)
{
    struct sym **parent = getscope();
    setscope(&ast->symtab);

    for (struct ast *node = ast->next; node; node = node->next)
        discard(cg(node));

    setscope(parent);
    return NOREG;
}

// Generate declaration
int cgdecl(struct ast *ast)
{
    if (ast->vtype.func && !ast->vtype.ptr)
    {
        fprintf(g_out, "%s:\n", ast->sv);
        fprintf(g_out, "\tpush %%rbp\n");
        fprintf(g_out, "\tmov %%rsp, %%rbp\n");
        fprintf(g_out, "\tsub %%rsp, %d\n", ast->left->symtab->stckoff);
        
        cg(ast->left);
                
        fprintf(g_out, "\tleave\n");
        fprintf(g_out, "\tret\n");
    }
}

int cg(struct ast *ast)
{
    switch (ast->type)
    {
        case A_BINOP: return cgbinop(ast);
        case A_ILIT:  return cgilit(ast);
        case A_CMPD:  return cgcmpd(ast);
        case A_DECL:  return cgdecl(ast);
        case A_IDENT: return cgident(ast);
    }

    return NOREG;
}