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

    // Parse function body if necessary
    if (ast->vtype.func && !ast->vtype.ptr)
    {
        expect(T_LBRACE);
        ast->left = cmpdstmt();
        expect(T_RBRACE);
    }

    addsym(ast->sv, ast->vtype);

    return ast;
}

struct ast *stmt()
{
    switch (g_tok.type)
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

    struct sym **parent = getscope();
    setscope(&ast->symtab);

    while (g_tok.type != T_EOF && g_tok.type != T_RBRACE)
    {
        ast->next = stmt();
        ast->next->prev = ast;

        ast = ast->next;

        expect(T_SEMI);
    }

    setscope(parent);
    return ret;
}