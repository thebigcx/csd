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
void func()
{
    char *name = strdup(token());
    EXPECT("(");
    EXPECT(")");

    cgfndef(name);
    EXPECT("{");

    block();

    EXPECT("}");
    cgfnend();

    free(name);
}

void decl(char *t)
{
    if (ISTOK(t, "fn")) func();
}

void stmt(char *t)
{
    cg(expr(t));
    EXPECT(";");
}