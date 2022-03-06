#include "cc.h"

struct type type()
{
    struct type t = { 0 };

    switch (g_tok.type)
    {
        case T_U32: t.sign = 0; t.size = 32; break;
    }

    scan();
    return t;
}