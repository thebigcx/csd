#include "cc.h"

#include <ctype.h>

struct tok scan()
{
    struct tok t = { 0 };

    char c;
    do c = fgetc(g_in); while (isblank(c));

    switch (c)
    {
        case '+': t.type = T_PLUS; return t;
    }

    if (isdigit(c))
    {
        t.iv = c - '0';
        t.type = T_ILIT;
    }

    return t;
}