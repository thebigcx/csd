#include "mcc.h"

#include <string.h>

struct ast *primary(char *t)
{
    struct ast *ast = NEW(struct ast);
    
    if (ilit(t)) ast->type = A_ILIT;
    else         ast->type = A_ID;

    ast->val  = strdup(t);
    return ast;
}

// Expression. t: current tok
struct ast *expr(char *t)
{
    struct ast *lhs = primary(t);
    if (!oper(t = token()))
    {
        tputbck(t);
        return lhs;
    }

    struct ast *ast = NEW(struct ast);

    ast->type = A_BINOP;
    ast->lhs  = lhs;
    ast->val  = strdup(t);
    ast->rhs  = expr(token());

    return ast;
}