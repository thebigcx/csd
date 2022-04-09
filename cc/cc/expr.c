#include "cc.h"

#include <stdlib.h>

struct ast *suffix();
struct ast *primary();

// Prefix of primary expression (*, &, etc.). Takes precedence over primary().
struct ast *prefix()
{
    struct ast *ast = NULL;

    switch (g_tok.type)
    {
        case T_STAR:
            ast = NEW(struct ast);
            ast->type = A_UNARY;
            ast->op = OP_DEREF;

            scan();
            ast->left = prefix();

            break;

        default:
            ast = suffix();
            break;
    }

    return ast;
}

// Suffix of primary expression ([], (), etc.). Takes precedence over prefix().
struct ast *suffix()
{
    struct ast *ast = NULL;

    switch (g_tok.type)
    {
        default:
            ast = primary();
            break;
    }

    return ast;
}

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
    return g_tok.type == T_SEMI || g_tok.type == T_RPAREN;
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
    struct ast *left = prefix();
    if (term())
        return left;

    struct ast *ast = NEW(struct ast);

    ast->left  = left;
    ast->mid   = arithop();
    ast->right = expr();

    return ast;
}