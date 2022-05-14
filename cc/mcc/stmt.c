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
        sym.type = gettype(t);
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

    type_t ftype;

    if (type(t)) {
        ftype = gettype(t);
        ftype.fn = 1;
        t = token();
    } else
        ftype = (type_t) { .sgn = 1, .sz = 4, .fn = 1 }; // int

    char *name = strdup(t);
    EXPECT("(");
    EXPECT(")");

    addsym((struct sym) { .name = name, .type = ftype, .class = SC_PUB });

    cgfndef(name, priv);
    frame();

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

void retrn()
{
    cgretrn(expr(token()));
}

void stmt(char *t)
{
    // Process declarations
    if (!stclass(t) && !s_frm) {
        cgscope(curtab()->stckoff);
        s_frm = 1;
    }

    if (ISTOK(t, "{"))          scope();
    else if (stclass(t))        var(t);
    else if (ISTOK(t, "retrn")) retrn();
    else                        cg(expr(t));

    EXPECT(";");
}