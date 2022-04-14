#include "asm.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const char *tokstrs[] = {
    [T_IDENT]  = "<identifier>",
    [T_NL]     = "<newline>",
    [T_EOF]    = "<EOF>",
    [T_ILIT]   = "<int literal>",
    [T_LBRACK] = "[",
    [T_RBRACK] = "]",
    [T_COMMA]  = ",",
    [T_LPAREN] = "(",
    [T_RPAREN] = ")",
    [T_STAR]   = "*",
    [T_PLUS]   = "+"
};

// Valid character in identifier
int validid(char c)
{
    return isalnum(c) || c == '_';
}

// TODO: turn into a lexing library
struct tok *scan()
{
    char buf[32] = { 0 };

    char c;
    do c = fgetc(g_in); while (c != -1 && (isspace(c) && c != '\n'));

    // fgetc() returned EOF, therefore finished reading file
    if (c == -1)
    {
        g_tok.type = T_EOF;
        return &g_tok;
    }
    else if (c == '\n')
    {
        g_tok.type = T_NL;
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
    else if (isdigit(c))
    {
        // Scan integer literal

        int buflen = 0;

        do buf[buflen++] = c; while (isdigit(c = fgetc(g_in)));
        buf[buflen] = 0;

        ungetc(c, g_in); // Last character was not part of integer literal

        g_tok.iv = strtoull(buf, NULL, 10);
        g_tok.type = T_ILIT;
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