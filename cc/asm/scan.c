#include "asm.h"

const char *tokstrs[] = {
    [T_EOF] = "<EOF>"
};

// TODO: turn into a lexing library
struct tok *scan()
{
    char buf[32] = { 0 };

    char c;
    do c = fgetc(g_in); while (c != -1 && isspace(c));

    // fgetc() returned EOF, therefore finished reading file
    if (c == -1)
    {
        g_tok.type = T_EOF;
        return &g_tok;
    }

    if (isalpha(c) || c == '_')
    {
        // Keyword or identifier

        int buflen = 0;
        
        do buf[buflen++] = c; while (validid(c = fgetc(g_in)));
        buf[buflen] = 0;

        ungetc(c, g_in); // Last character was not part of identifier

        //if (!strcmp("extern", buf)) g_tok.type = T_EXTERN;
        //else
        {
            g_tok.type = T_IDENT;
            g_tok.sv = strdup(buf);
        }
    }
    else
    {
        // Nice algorithm to find the longest matching token string.
        // For e.g.: - will be T_MINUS, whereas -> will be T_ARROW

        int n = 0, t = -1;

    nextch:
        buf[n++] = c;
        for (int i = 0; i < TOKCNT; i++)
        {
            if (!strncmp(buf, tokstrs[i], n))
            {
                t = i;
                c = fgetc(g_in);
                goto nextch;
            }
        }

        ungetc(c, g_in);
        g_tok.type = t;
    }
}