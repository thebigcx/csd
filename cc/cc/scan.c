#include "cc.h"

#include <ctype.h>
#include <string.h>

// Valid character in identifier
int validid(char c)
{
    return isalnum(c) || c == '_';
}

struct tok *scan()
{
    char buf[32];

    char c;
    do c = fgetc(g_in); while (isblank(c));

    switch (c)
    {
        case '+': g_tok.type = T_PLUS;  return &g_tok;
        case ';': g_tok.type = T_SEMI;  return &g_tok;
        case ':': g_tok.type = T_COLON; return &g_tok;
    }

    if (isdigit(c))
    {
        g_tok.iv = c - '0';
        g_tok.type = T_ILIT;
    }
    else if (isalpha(c) || c == '_')
    {
        // Keyword or identifier

        int buflen = 0;
        
        do buf[buflen++] = c; while (validid(c = fgetc(g_in)));
        buf[buflen] = 0;

        ungetc(c, g_in); // Last character was not part of identifier

        if (!strcmp("let", buf)) g_tok.type = T_LET;
        else if (!strcmp("u32", buf)) g_tok.type = T_U32;
        else
        {
            g_tok.type = T_IDENT;
            g_tok.sv = strdup(buf);
        }
    }

    return &g_tok;
}

int qualif()
{
    return g_tok.type == T_PUB;
}

struct tok expect(int t)
{
    if (g_tok.type != t)
    {
        printf("Expectation %d, got %d\n", t, g_tok.type);
        // Crash
    }

    struct tok tok = g_tok;
    scan();
    return tok;
}