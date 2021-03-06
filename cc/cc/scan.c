#include "cc.h"

#include <ctype.h>
#include <string.h>

const char *tokstrs[] = {
    [T_SEMI]   = ";",
    [T_ILIT]   = "<int literal>",
    [T_PLUS]   = "+",
    [T_PUB]    = "pub",
    [T_FN]     = "fn",
    [T_LET]    = "let",
    [T_IDENT]  = "<identifier>",
    [T_COLON]  = ":",
    [T_U32]    = "u32",
    [T_EQ]     = "=",
    [T_LPAREN] = "(",
    [T_RPAREN] = ")",
    [T_COMMA]  = ",",
    [T_STAR]   = "*",
    [T_LBRACE] = "{",
    [T_RBRACE] = "}",
    [T_IF]     = "if",
    [T_LT]     = "<",
    [T_RET]    = "ret",
    [T_AMP]    = "&",
    [T_EXTERN] = "extern",
    [T_ARROW]  = "->",
    [T_EOF]    = "<EOF>"
};

// Valid character in identifier
int validid(char c)
{
    return isalnum(c) || c == '_';
}

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

    if (isdigit(c))
    {
        // Scan integer literal

        int buflen = 0;

        do buf[buflen++] = c; while (isdigit(c = fgetc(g_in)));
        buf[buflen] = 0;

        ungetc(c, g_in); // Last character was not part of integer literal

        g_tok.iv = strtoull(buf, NULL, 10);
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
        else if (!strcmp("fn", buf))  g_tok.type = T_FN;
        else if (!strcmp("u32", buf)) g_tok.type = T_U32;
        else if (!strcmp("if", buf))  g_tok.type = T_IF;
        else if (!strcmp("ret", buf)) g_tok.type = T_RET;
        else if (!strcmp("pub", buf)) g_tok.type = T_PUB;
        else if (!strcmp("extern", buf)) g_tok.type = T_EXTERN;
        else
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
        printf("Expected %s, got %s\n", TOKSTR(t), TOKSTR(g_tok.type));
        // Crash
    }

    struct tok tok = g_tok;
    scan();
    return tok;
}