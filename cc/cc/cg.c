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

// Generate a new label unique in this translation unit
int label()
{
    static int l = 0;
    return l++;
}

// Generate the code for a variable read/write e.g. [rsp - 10] or [label]
void cgvarloc(struct sym *sym)
{
    if (sym->global)
        fprintf(g_out, "[%s]", sym->name);
    else
        fprintf(g_out, "[rsp - %d]", sym->stckoff);
}

int cgident(struct ast *ast)
{
    int r = ralloc();

    fprintf(g_out, "\tmov %s, ", regs[r]);
    cgvarloc(lookup(ast->sv));
    fprintf(g_out, "\n");

    return r;
}

int cgadd(int r1, int r2)
{
    fprintf(g_out, "\tadd %s, %s\n", regs[r1], regs[r2]);
    rfree(r2);
    return r1;
}

int cgcmp(int r1, int r2, int op)
{
    int r3 = ralloc();
    fprintf(g_out, "\tcmp %s, %s\n", regs[r1], regs[r2]);

    switch (op)
    {
        case OP_LT: fprintf(g_out, "\tsetl %s\n", regs[r3]); break;
    }

    rfree(r1);
    rfree(r2);
    return r3;
}

// Dereference operator.
int cgderef(struct ast *ast)
{
    int r1 = cg(ast);
    int r2 = ralloc();
    
    fprintf(g_out, "\tmov %s, [%s]\n", regs[r2], regs[r1]);

    rfree(r1);
    return r2;
}

int cgunary(struct ast *ast)
{
    switch (ast->op)
    {
        case OP_DEREF: return cgderef(ast->left);
    }
}

int cgassign(struct ast *left, int r)
{
    // Dereference
    if (left->type == A_UNARY)
    {
        int adr = cg(left->left);
        fprintf(g_out, "\tmov [%s], %s\n", regs[adr], regs[r]);
        rfree(adr);
    }
    else
    {
        fprintf(g_out, "\tmov ");
        cgvarloc(lookup(left->sv));
        fprintf(g_out, ", %s\n", regs[r]);
    }

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
        
        case OP_LT:
            return cgcmp(r1, r2, ast->mid->op);
    }
}

// Generate code for compound statement
int cgcmpd(struct ast *ast)
{
    struct symtab *parent = getscope();
    setscope(&ast->symtab);

    if (ast->symtab.stcksz)
    {
        fprintf(g_out, "\tpush %%rbp\n");
        fprintf(g_out, "\tmov %%rsp, %%rbp\n");
        fprintf(g_out, "\tsub %%rsp, %d\n", ast->symtab.stcksz);
    } 
    
    for (struct ast *node = ast->next; node; node = node->next)
        discard(cg(node));

    if (ast->symtab.stcksz)            
        fprintf(g_out, "\tleave\n");

    setscope(parent);
    return NOREG;
}

// Generate if statement code
int cgif(struct ast *ast)
{
    int l1 = label();

    int r = cg(ast->left); // Condition

    fprintf(g_out, "\ttest %s, %s\n", regs[r], regs[r]);
    fprintf(g_out, "\tjz L%d\n", l1);

    discard(cg(ast->mid));

    fprintf(g_out, "L%d:\n", l1);

    return NOREG;
}

// Generate declaration
int cgdecl(struct ast *ast)
{
    if (ast->vtype.func && !ast->vtype.ptr)
    {
        fprintf(g_out, "%s:\n", ast->sv);
        cg(ast->left);
        fprintf(g_out, "\tret\n");
    }

    return NOREG;
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
        case A_UNARY: return cgunary(ast);
        case A_IF:    return cgif(ast);
    }

    return NOREG;
}