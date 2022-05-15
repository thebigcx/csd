#include "mcc.h"

#include <string.h>

struct ast *primary(char*);
struct ast *post(struct ast*, char*);

struct ast *pre(char *t)
{
    if (star(t)) {
        struct ast *ast = NEW(struct ast);

        ast->type = A_DEREF;
        ast->lhs  = pre(token());
        
        // Decrement pointer
        ast->vt = ast->lhs->vt;
        ast->vt.ptr--;

        return ast;
    } else if (ISTOK(t, "&")) {
        struct ast *lhs = pre(token());

        /* Cancel out &* */
        if (lhs->type == A_DEREF) return lhs->lhs;

        struct ast *ast = NEW(struct ast);

        ast->type = A_ADDR;
        ast->lhs  = lhs;

        ast->vt   = ast->lhs->vt;
        ast->vt.ptr++;

        return ast;
    } else if (ISTOK(t, "!") || ISTOK(t, "~") || ISTOK(t, "-")) {
        struct ast *ast = NEW(struct ast);

        ast->type = A_UNARY;
        ast->val  = strdup(t);
        ast->lhs  = pre(token());
        ast->vt   = ast->lhs->vt;

        return ast;
    } 

    struct ast *br = primary(t);
    return post(br, token());
    //return post(primary(t), token());
}

// Post-fix
struct ast *post(struct ast *br, char *t)
{
    if (ISTOK(t, "(")) {
        struct ast *ast = NEW(struct ast);
        
        ast->type = A_CALL;
        ast->lhs  = br;
        ast->vt   = br->vt; // TODO

        ast->vt.fn = 0;

        t = token();

        // Parse parameters
        struct ast **p = &ast;
        while (!ISTOK(t, ")")) {
            struct ast **prv = p;

            p = &(*p)->nxt;
            *p = expr(t);
            (*p)->prv = *prv;

            if (ISTOK(t = token(), ",")) t = token();
        }

        ast->prv = *p;
        return ast;
    }

    tputbck(t);
    return br;
}

struct ast *primary(char *t)
{
    struct ast *ast = NEW(struct ast);
    
    ast->val  = strdup(t);
    
    if (ilit(t)) {
        ast->type = A_ILIT;
        ast->vt   = VT_INT;
    } else {
        ast->type = A_ID;
        ast->vt   = lookup(ast->val)->type;
    }

    return ast;
}

// Expression. t: current tok
struct ast *expr(char *t)
{
    struct ast *lhs = pre(t);
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
    ast->vt   = lhs->vt;

    return ast;
}