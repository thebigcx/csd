#include "mcc.h"

static int s_frm = 0; // Stack frame?

/* Block of statements */
void block()
{
    char *t;
    while (!ISTOK(t = token(), "}"))
        stmt(t);

    tputbck(t);
}

void frame()
{
    s_frm = 0;
    newscope();
}

// var() implementation, par = parameter offset
static void _var(char *t, int class, int *par)
{
    struct sym sym = { .class = class };

    if (type(t)) {
        if (*t != 'u') sym.type.sgn = 1;
        else t++;

            if (!strcmp(t, "char"))  sym.type.sz = 1;
        else if (!strcmp(t, "int"))  sym.type.sz = 4;
        else if (!strcmp(t, "long")) sym.type.sz = 8;

        t = token();
    } else
        sym.type = (type_t) { .sz = 4, .sgn = 1 }; // int

    sym.name = strdup(t);

    if (par) {
        sym.off = *par;
        *par -= tysize(&sym.type);
        addsymoff(sym);
    } else
        addsym(sym);

    cgvardef(&sym);
}

/* Function definition */
void func(char *t)
{
    int priv = 0;

    // Private declaration
    if (ISTOK(t, "priv")) {
        priv = 1;
        t = token();
    }

    if (type(t)) t = token(); // TODO: return type

    char *name = strdup(t);
    EXPECT("(");
    EXPECT(")");

    cgfndef(name, priv);
    frame();
    // TODO: parameters

    int offset = -16;
    while (strcmp(t = token(), "{")) {
        _var(t, SC_AUTO, &offset);
        EXPECT(";");
    }

    block();

    EXPECT("}");
    retscope();
    cgleave();
    cgfnend();

    free(name);
}

/* Variable declaration */
void var(char *t)
{
    // Storage class
    int class = ISTOK(t, "auto")   ? SC_AUTO
              : ISTOK(t, "static") ? SC_STAT
              : ISTOK(t, "extrn")  ? SC_EXTRN
              : ISTOK(t, "pub")    ? SC_PUB : SC_REG;

    _var(t = token(), class, NULL);
}

void decl(char *t)
{
    if (stclass(t)) var(t);
    else            func(t);

    EXPECT(";");
}

void scope()
{
    frame();
    block();

    cgleave();
    retscope();

    EXPECT("}");
}

void stmt(char *t)
{
    // Process declarations
    if (!stclass(t) && !s_frm) {
        cgscope(curtab()->stckoff);
        s_frm = 1;
    }

    if (ISTOK(t, "{"))   scope();
    else if (stclass(t)) var(t);
    else                 cg(expr(t));

    EXPECT(";");
}