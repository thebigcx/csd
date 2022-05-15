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

static const char *s_r8[] = {
    [RAX] = "al",
    [RBX] = "bl",
    [RCX] = "cl",
    [RDX] = "dl",
    [R8]  = "r8l",
    [R9]  = "r9l",
    [R10] = "r10l",
    [R11] = "r11l",
    [R12] = "r12l",
    [R13] = "r13l",
    [R14] = "r14l",
    [R15] = "r15l"
};

static const char *s_r16[] = {
    [RAX] = "ax",
    [RBX] = "bx",
    [RCX] = "cx",
    [RDX] = "dx",
    [R8]  = "r8w",
    [R9]  = "r9w",
    [R10] = "r10w",
    [R11] = "r11w",
    [R12] = "r12w",
    [R13] = "r13w",
    [R14] = "r14w",
    [R15] = "r15w"
};

static const char *s_r32[] = {
    [RAX] = "eax",
    [RBX] = "ebx",
    [RCX] = "ecx",
    [RDX] = "edx",
    [R8]  = "r8d",
    [R9]  = "r9d",
    [R10] = "r10d",
    [R11] = "r11d",
    [R12] = "r12d",
    [R13] = "r13d",
    [R14] = "r14d",
    [R15] = "r15d"
};

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

// Get register
const char *reg(int n, type_t t)
{
    switch (tysize(&t))
    {
        case 1: return s_r8[n];
        case 2: return s_r16[n];
        case 4: return s_r32[n];
        case 8: return s_r64[n];
    }

    return NULL;
}

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
    static int l = 1;
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
        case SC_REG:   fprintf(s_out, "%s", reg(s->off, s->type)); break;
    }
}

// Assignment expression
int cgassign(struct ast *ast)
{
    int rhs = cg(ast->rhs);

    if (ast->lhs->type == A_DEREF) {
        // Dereference pointer
        int ad = cg(ast->lhs->lhs);
        fprintf(s_out, "\tmov [%s], %s\n", reg(ad, ast->lhs->vt), reg(rhs, ast->lhs->vt));
        rfree(ad);
    } else {
        // Lookup symbol
        struct sym *s = lookup(ast->lhs->val);

        fprintf(s_out, "\tmov ");
        cgaccess(s);
        fprintf(s_out, ", %s ; <%s>\n", reg(rhs, ast->lhs->vt), s->name);
    }

    return rhs;
}

const char *cmp(char *op)
{
    if (!strcmp(op, "<"))  return "setl";
    if (!strcmp(op, ">"))  return "setg";
    if (!strcmp(op, "<=")) return "setle";
    if (!strcmp(op, ">=")) return "setge";
    if (!strcmp(op, "==")) return "sete";
    if (!strcmp(op, "!=")) return "setne";

    return NULL;
}

int cgcmp(struct ast *ast, int lhs, int rhs)
{
    const char *inst = cmp(ast->val);

    fprintf(s_out, "\tcmp %s, %s\n", reg(lhs, ast->vt), reg(rhs, ast->vt));
    fprintf(s_out, "\t%s %s\n", inst, reg(rhs, (type_t) { .sz = 1 }));
    fprintf(s_out, "\tmovzx %s, %s\n", reg(lhs, ast->vt), reg(rhs, (type_t) { .sz = 1 }));

    rfree(rhs);
    return lhs;
}

// Lazy evaluation
int cglog(struct ast *ast)
{
    int l = label();
    
    int lhs = cg(ast->lhs);
    fprintf(s_out, "\ttest %s, %s\n", reg(lhs, ast->vt), reg(lhs, ast->vt));

    /* If &&, skip rhs if 0, if ||, skip rhs if 1 */
    if (!strcmp(ast->val, "&&"))
        fprintf(s_out, "\tjz L%d\n", l);
    if (!strcmp(ast->val, "||"))
        fprintf(s_out, "\tjnz L%d\n", l);

    int rhs = cg(ast->rhs);
    fprintf(s_out, "\ttest %s, %s\n", reg(rhs, ast->vt), reg(rhs, ast->vt));
    
    fprintf(s_out, ":L%d\n", l);
    fprintf(s_out, "\tsetne %s\n", reg(rhs, (type_t) { .sz = 1 }));
    fprintf(s_out, "\tmovzx %s, %s\n", reg(lhs, ast->vt), reg(rhs, (type_t) { .sz = 1 }));

    rfree(rhs);
    return lhs;
}

// Generate binary operation
int cgbinop(struct ast *ast)
{
    if (!strcmp(ast->val, "=")) return cgassign(ast);

    if (!strcmp(ast->val, "&&") || !strcmp(ast->val, "||"))
        return cglog(ast);

    int rhs = cg(ast->rhs);
    int lhs = cg(ast->lhs);

    if (cmp(ast->val)) return cgcmp(ast, lhs, rhs);

    const char *inst = NULL;

    if (!strcmp(ast->val, "+")) inst = "add";
    if (!strcmp(ast->val, "-")) inst = "sub";
    if (!strcmp(ast->val, "&")) inst = "and";
    if (!strcmp(ast->val, "|")) inst = "or";
    if (!strcmp(ast->val, "^")) inst = "xor";

    //if (!strcmp(ast->val, "&&")) inst = "and";
    //if (!strcmp(ast->val, "||")) inst = "or";

    fprintf(s_out, "\t%s %s, %s\n", inst, reg(lhs, ast->lhs->vt), reg(rhs, ast->rhs->vt));

    rfree(rhs);
    return lhs;
}

int cgunary(struct ast *ast)
{
    int lhs = cg(ast->lhs);
    int log = !strcmp(ast->val, "!"); // Logical

    const char *inst = NULL;

    if (!strcmp(ast->val, "~")|| log) inst = "not";
    if (!strcmp(ast->val, "-")) inst = "neg";

    fprintf(s_out, "\t%s %s\n", inst, reg(lhs, ast->lhs->vt));

    if (log)
        fprintf(s_out, "\tand %s, 1\n", reg(lhs, ast->lhs->vt));

    return lhs;
}

// Generate int literal
int cgilit(struct ast *ast)
{
    int r = ralloc();
    fprintf(s_out, "\tmov %s, %s\n", reg(r, ast->vt), ast->val);
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

    fprintf(s_out, "\tmov %s, ", reg(r, s->type));
    cgaccess(s);
    fprintf(s_out, " ; <%s>\n", s->name);

    return r;
}

/* Generate dereference */
int cgderef(struct ast *ast)
{
    int r2 = cg(ast->lhs);
    int r1 = ralloc();

    fprintf(s_out, "\tmov %s, [%s]\n", reg(r1, ast->vt), reg(r2, ast->lhs->vt));

    rfree(r2);
    return r1;
}

/* Generate address-of */
int cgaddr(struct ast *ast)
{
    int r1 = ralloc();

    struct sym *s = lookup(ast->lhs->val);

    fprintf(s_out, "\tlea %s, ", reg(r1, ast->vt));
    cgaccess(s);
    fprintf(s_out, " ; <%s>\n", s->name);

    return r1;
}

int cgcall(struct ast *ast)
{
    // Generate parameters (reverse order)
    for (struct ast *p = ast->prv; p != ast; p = p->prv)
        p->l1 = cg(p);

    // Push them
    for (struct ast *p = ast->prv; p != ast; p = p->prv) {
        fprintf(s_out, "\tpush %s\n", reg(p->l1, p->vt));
        rfree(p->l1);
    }

    if (ast->lhs->type == A_ID) {
        fprintf(s_out, "\tcall %s\n", ast->lhs->val);
    } else {
        int ad = cg(ast->lhs);
        fprintf(s_out, "\tcall %s\n", reg(ad, ast->lhs->vt));
        rfree(ad);
    }

    // Restore stack
    for (struct ast *p = ast->nxt; p; p = p->nxt)
        fprintf(s_out, "\tpop %s\n", reg(RBX, p->vt));

    type_t ret = ast->lhs->vt;
    ret.fn = 0;

    // Move return value
    int r = ralloc();
    if (r != RAX)
        fprintf(s_out, "\tmov %s, %s\n", reg(r, ast->vt), reg(RAX, ret));
    return r;
}

int cg(struct ast *ast)
{
    switch (ast->type) {
        case A_BINOP: return cgbinop(ast);
        case A_ILIT:  return cgilit(ast);
        case A_ID:    return cgid(ast);
        case A_DEREF: return cgderef(ast);
        case A_CALL:  return cgcall(ast);
        case A_UNARY: return cgunary(ast);
        case A_ADDR:  return cgaddr(ast);
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
    fprintf(s_out, "\tleave\n");
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

    if (curtab()->syms)
        fprintf(s_out, "\tleave\n");
}

void cgretrn(struct ast *e)
{
    // Return expression
    int r = cg(e);
    if (r != RAX)
        fprintf(s_out, "\tmov rax, %s\n", reg(r, e->vt));

    fprintf(s_out, "\tjmp L%d\n", s_elbl);
}

void cgif(struct ast *ast)
{
    ast->l1 = label();

    int r = cg(ast->lhs);

    fprintf(s_out, "\ttest %s, %s\n", reg(r, ast->lhs->vt), reg(r, ast->lhs->vt));
    fprintf(s_out, "\tjz L%d\n", ast->l1);

    rfree(r);
}

void cgifelse(struct ast *ast)
{
    ast->l2 = label();
    
    fprintf(s_out, ":L%d\n", ast->l1);
}

void cgifend(struct ast *ast)
{
    fprintf(s_out, ":L%d\n", ast->l2 ? ast->l2 : ast->l1);
}

void cgwhile(struct ast *ast)
{
    ast->l1 = label();
    ast->l2 = label();

    fprintf(s_out, ":L%d\n", ast->l1);
    
    int r = cg(ast->lhs);
    fprintf(s_out, "\ttest %s, %s\n", reg(r, ast->lhs->vt), reg(r, ast->lhs->vt));
    fprintf(s_out, "\tjz L%d\n", ast->l2);
}

void cgwhileend(struct ast *ast)
{
    fprintf(s_out, "\tjmp L%d\n", ast->l1);
    fprintf(s_out, ":L%d\n", ast->l2);
}

void cgdiscard(int r)
{
    if (r != NREG) rfree(r);
}

void cgbyte(char c)
{
    fputc(c, s_out);
}