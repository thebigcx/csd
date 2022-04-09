#include "cc.h"

struct type type()
{
    struct type t = { 0 };

    // Pointer count
    while (g_tok.type == T_STAR)
    {
        t.ptr++;
        scan();
    }

    if (g_tok.type == T_FN)
    {
        t.func = 1;

        scan();

        // Parse parameter list with linked-list magic
        expect(T_LPAREN);

        struct type **param = &t.param;
        struct type **next = &t.next;

        while (g_tok.type != T_RPAREN)
        {
            *param = NEW(struct type);
            **param = type();

            param = next;
            next = &(*next)->next;

            // Comma-separated
            if (g_tok.type != T_RPAREN) expect(T_COMMA);
        }

        expect(T_RPAREN);
    }
    else
    {
        switch (g_tok.type)
        {
            case T_U32: t.sign = 0; t.size = 4; break;
        }
        scan();
    }

    return t;
}

unsigned int typesize(struct type *t)
{
    return t->size;
}