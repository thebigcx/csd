#include "cc.h"

#include <stdlib.h>

struct ast *suffix();
struct ast *primary();
struct ast *prefix();

// Address-of: trivial for identifiers, but in the case of
// arrays and structs (which are translated to *(&arr + 4) for e.g.),
// it will remove the dereference, therefore returning a memory address.
struct ast *addrof()
{
    expect(T_AMP);
    struct ast *left = prefix();

    if (left->type == A_UNARY && left->op == OP_DEREF)
    {
        // TODO: free the node
        //struct ast *tmp = left;
        //left = left->left;
        //free(tmp);
        
        // &* cancels out.
        return left->left;
    }
    else
    {
        struct ast *ast = NEW(struct ast);
        ast->type = A_UNARY;
        ast->op   = OP_ADDR;
        ast->left = left;
        return ast;
    }
}

// Dereference unary operator.
struct ast *deref()
{
    expect(T_STAR);

    struct ast *ast = NEW(struct ast);
    ast->type = A_UNARY;
    ast->op   = OP_DEREF;
    ast->left = prefix();
    return ast;
}

// Prefix of primary expression (*, &, etc.). Takes precedence over primary().
struct ast *prefix()
{
    struct ast *ast = NULL;

    switch (g_tok.type)
    {
        case T_STAR: return deref();
        case T_AMP:  return addrof();

        default:
            ast = suffix(primary());
            break;
    }

    return ast;
}

// Call expression
struct ast *callexpr(struct ast *left)
{
    expect(T_LPAREN);
    expect(T_RPAREN);

    struct ast *ast = NEW(struct ast);
    ast->type = A_CALL;
    ast->left = left;

    return ast;
}

// Suffix of primary expression ([], (), etc.). Takes precedence over prefix().
struct ast *suffix(struct ast *left)
{
    switch (g_tok.type)
    {
        case T_LPAREN: return callexpr(left);
        default: return left;
    }
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
        case T_LT:   ast->op = OP_LT;     break;
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
    
    ast->type  = A_BINOP;
    ast->left  = left;
    ast->mid   = arithop();
    ast->right = expr();

    return ast;
}