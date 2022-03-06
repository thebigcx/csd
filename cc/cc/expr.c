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
        
        case T_IDENT:
            if (!lookup(g_tok.sv))
                printf("Undeclared symbol '%s'\n", g_tok.sv);

            ast->type = A_IDENT;
            ast->sv = g_tok.sv;
            break;
    }

    scan();
    return ast;
}

int term()
{
    return g_tok.type == T_SEMI;
}

// Arithmetic operator, e.g. + - * /
struct ast *arithop()
{
    struct ast *ast = NEW(struct ast);
    ast->type = A_OP;

    switch (g_tok.type)
    {
        case T_PLUS: ast->op = OP_ADD;    break;
        case T_EQ:   ast->op = OP_ASSIGN; break;
    }

    scan();
    return ast;
}

struct ast *expr()
{
    struct ast *left = primary();
    if (term())
        return left;

    struct ast *ast = NEW(struct ast);

    ast->left  = left;
    ast->mid   = arithop();
    ast->right = expr();

    return ast;
}