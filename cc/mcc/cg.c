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
static int s_elbl = 0; // End label of current function

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

    if (i >= RCNT)
        printf("Out of registers!\n");

    s_rmap[i] = 1;
    return i;
}

// Free register
void rfree(int r)
{
    s_rmap[r] = 0;
}

// Generate a unique label
static int label()
{
    static int l = 0;
    return l++;
}

// Print a stack offset
static void printoff(int off)
{
         if (off < 0) fprintf(s_out, " - %d", -off);
    else if (off > 0) fprintf(s_out, " + %d", off);
}

// Access a variable
void cgaccess(struct sym *s)
{
    switch (s->class) {
        case SC_AUTO:
            fprintf(s_out, "u%d [rsp", tysize(&s->type) * 8);
            printoff(-symstckoff(s));
            fprintf(s_out, "]");
            break;

        case SC_PUB:
        case SC_STAT:
        case SC_EXTRN: fprintf(s_out, "u%d [%s]", tysize(&s->type) * 8, s->name); break;
        case SC_REG:   fprintf(s_out, "%s", s_r64[s->off]); break;
    }
}

// Assignment expression
int cgassign(struct ast *ast)
{
    int rhs = cg(ast->rhs);

    if (ast->lhs->type == A_DEREF) {
        // Dereference pointer
        int ad = cg(ast->lhs->lhs);
        fprintf(s_out, "\tmov [%s], %s\n", s_r64[ad], s_r64[rhs]);
        rfree(ad);
    } else {
        // Lookup symbol
        struct sym *s = lookup(ast->lhs->val);

        fprintf(s_out, "\tmov ");
        cgaccess(s);
        fprintf(s_out, ", %s : <%s>\n", s_r64[rhs], s->name);
    }

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

void cgvardef(struct sym *s)
{
    if (s->class == SC_REG) {
        s->off = ralloc();
    }

    if (s->class == SC_PUB) fprintf(s_out, "\tglobal %s\n", s->name);
    if (s->class == SC_EXTRN) fprintf(s_out, "\textern %s\n", s->name);

    if (s->class == SC_PUB || s->class == SC_STAT) {
        fprintf(s_out, ":%s\n", s->name);
        fprintf(s_out, "\tzero %d\n", tysize(&s->type));
    }

    // TODO: data initialization
}

int cgid(struct ast *ast)
{
    int r = ralloc();

    struct sym *s = lookup(ast->val);

    fprintf(s_out, "\tmov %s, ", s_r64[r]);
    cgaccess(s);
    fprintf(s_out, " ; <%s>\n", s->name);

    return r;
}

/* Generate dereference */
int cgderef(struct ast *ast)
{
    int r1 = ralloc();
    int r2 = cg(ast->lhs);

    fprintf(s_out, "\tmov %s, [%s]\n", s_r64[r1], s_r64[r2]);

    rfree(r2);
    return r1;
}

int cg(struct ast *ast)
{
    switch (ast->type) {
        case A_BINOP: return cgbinop(ast);
        case A_ILIT:  return cgilit(ast);
        case A_ID:    return cgid(ast);
        case A_DEREF: return cgderef(ast);
    }

    return NREG;
}

void cgfile(FILE *out)
{
    s_out = out;
}

void cgfndef(char *name, int priv)
{
    if (!priv) fprintf(s_out, "\tglobal %s\n", name);
    fprintf(s_out, ":%s\n", name);

    s_elbl = label();
}

void cgfnend()
{
    fprintf(s_out, ":L%d\n", s_elbl);
    fprintf(s_out, "\tret\n");
}

void cgscope(size_t s)
{
    fprintf(s_out, "\tpush rbp\n");
    fprintf(s_out, "\tmov rbp, rsp\n");
    fprintf(s_out, "\tsub rsp, %ld\n", s);
}

void cgleave()
{
    // Free all 'register' symbols
    for (struct sym *s = curtab()->syms; s; s = s->nxt)
        if (s->class == SC_REG) rfree(s->off);

    fprintf(s_out, "\tleave\n");
}

void cgretrn(struct ast *e)
{
    // Return expression
    int r = cg(e);
    if (r != RAX)
        fprintf(s_out, "\tmov rax, %s\n", s_r64[r]);

    fprintf(s_out, "\tjmp L%d\n", s_elbl);
}