#include "mcc.h"

/* Block of statements */
void block()
{
    char *t;
    while (!ISTOK(t = token(), "}"))
        stmt(t);

    tputbck(t);
}

/* Function definition */
void func(char *t)
{
    if (type(t)) t = token(); // TODO: return type

    char *name = strdup(t);
    EXPECT("(");
    EXPECT(")");

    cgfndef(name);
    EXPECT("{");

    block();

    EXPECT("}");
    cgfnend();

    free(name);
}

static const char *s_stclas[] = {
    [SC_AUTO]  = "auto",
    [SC_STAT]  = "static",
    [SC_EXTRN] = "extrn",
    [SC_REG]   = "regis"
};

/* Variable declaration */
void var(char *t)
{
    struct sym sym = { 0 };

    // Storage class
    sym.class = ISTOK(t, "auto")   ? SC_AUTO
              : ISTOK(t, "static") ? SC_STAT
              : ISTOK(t, "extrn")  ? SC_EXTRN : SC_REG;

    if (type(t = token())) {
        if (*t != 'u') sym.type.sgn = 1;
        else t++;

            if (!strcmp(t, "char"))  sym.type.sz = 1;
        else if (!strcmp(t, "int"))  sym.type.sz = 4;
        else if (!strcmp(t, "long")) sym.type.sz = 8;

        t = token();
    } else
        sym.type = (type_t) { .sz = 4, .sgn = 1 }; // int

    sym.name = strdup(t);

    addsym(sym);
}

void decl(char *t)
{
    if (stclass(t)) var(t);
    else            func(t);

    EXPECT(";");
}

void stmt(char *t)
{
    if (ISTOK(t, "{")) { block(); EXPECT("}"); }
    else if (stclass(t)) var(t);
    else                 cg(expr(t));

    EXPECT(";");
}