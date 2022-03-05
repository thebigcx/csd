#include "cc.h"

#include <stdlib.h>

// Primary expression, e.g. int literal, identifier
struct ast *primary()
{
    struct ast *ast = NEW(struct ast);

    switch (g_tok.type)
    {
        case T_ILIT:
            ast->type = A_ILIT;
            ast->iv   = g_tok.iv;
            break;
    }

    scan();
    return ast;
}

// Arithmetic operator, e.g. + - * /
struct ast *arithop()
{
    struct ast *ast = NEW(struct ast);
    ast->type = A_OP;

    switch (g_tok.type)
    {
        case T_PLUS: ast->op = OP_ADD; break;
    }

    scan();
    return ast;
}

struct ast *expr()
{
    struct ast *ast = NEW(struct ast);

    ast->left  = primary();
    ast->mid   = arithop();
    ast->right = primary();

    return ast;
}