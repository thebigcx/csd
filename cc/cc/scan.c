#include "cc.h"

#include <ctype.h>

struct tok scan()
{
    char c;
    do c = fgetc(g_in); while (isblank(c));

    switch (c)
    {
        case '+': g_tok.type = T_PLUS; return g_tok;
    }

    if (isdigit(c))
    {
        g_tok.iv = c - '0';
        g_tok.type = T_ILIT;
    }

    return g_tok;
}