#include "cc.h"

// Variable declaration - can be a function (functions are variables)
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

// Linked-list magic to construct a compound statement node
struct ast *cmpdstmt()
{
    struct ast *ast = NEW(struct ast), *ret = ast;
    ast->type = A_CMPD;

    while (g_tok.type != T_EOF)
    {
        ast->next = stmt();
        ast->next->prev = ast;

        ast = ast->next;

        expect(T_SEMI);
    }

    return ret;
}