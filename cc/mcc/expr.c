#include "mcc.h"

#include <string.h>

struct ast *primary(char *t)
{
    struct ast *ast = NEW(struct ast);
    
    ast->type = A_ILIT;
    ast->val  = strdup(t);

    return ast;
}

// Expression. t: current tok
struct ast *expr(char *t)
{
    struct ast *lhs = primary(t);
    if (!oper(t = token()))
        return lhs;

    struct ast *ast = NEW(struct ast);

    ast->type = A_BINOP;
    ast->lhs  = lhs;
    ast->val  = strdup(t);
    ast->rhs  = expr(token());

    return ast;
}