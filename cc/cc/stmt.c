#include "cc.h"

struct ast *decl()
{
    struct ast *ast = NEW(struct ast);
    ast->type = A_DECL;

    expect(T_LET);
    
    ast->sv = expect(T_IDENT).sv;

    expect(T_COLON);

    ast->vtype = type();
    return ast;
}

struct ast *stmt()
{
    switch (scan()->type)
    {
        case T_LET: return decl();
        default: return expr();
    }
}