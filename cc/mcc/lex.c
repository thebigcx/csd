#include "mcc.h"

#include <string.h>
#include <ctype.h>

static FILE *s_f      = NULL;
static char *s_putbck = NULL;

// List of symbol tokens
static char *s_toks[] = {
    "+", "-", "*", "/", "=",
    "(", ")", "{", "}",
    ";"
};

void lex_file(FILE *f)
{
    s_f = f;
}

// Skip spaces
static int skipspc(char *p)
{
    while (isspace(*p = fgetc(s_f)));
    return *p == EOF;
}

// Scan an identifier or ilit
static void scanid(char *buf)
{
    while (isalnum(*(++buf) = fgetc(s_f)) && *buf != EOF);
    
    ungetc(*buf, s_f);
    *buf = 0;
}

// Is token?
static int istok(char *s, int l)
{
    for (int i = 0; i < ALEN(s_toks); i++)
        if (!strncmp(s, s_toks[i], l)) return 1;
    return 0;
}

// Scan one or more non-alnum chars
static void scansym(char *buf)
{
    char *ptr = buf + 1; // First char already scanned

    for (int i = 1; istok(buf, i); *ptr++ = fgetc(s_f), i++);

    // Last character not valid
    ptr--;
    ungetc(*ptr, s_f);
    *ptr = 0;
}

char *token()
{
    // Return putback token
    if (s_putbck) {
        char *t = s_putbck;
        s_putbck = NULL;
        return t;
    }

    static char buf[128];
    char *ptr = buf;

    if (skipspc(ptr))
        return NULL;

    if (isalnum(*ptr)) // Ident or ilit
        scanid(ptr);
    else               // Scan symbol(s)
        scansym(ptr);

    return buf;
}

int oper(char *t)
{
    return t && (*t == '+' || *t == '-' || *t == '=');
}

int eof()
{
    return feof(s_f);
}

void tputbck(char *t)
{
    s_putbck = t;
}

int stclass(char *t)
{
    return ISTOK(t, "auto")   || ISTOK(t, "extrn")
        || ISTOK(t, "static") || ISTOK(t, "regis") || ISTOK(t, "pub");
}

int type(char *t)
{
    return ISTOK(t, "int")   || ISTOK(t, "uint") || ISTOK(t, "char")
        || ISTOK(t, "uchar") || ISTOK(t, "long") || ISTOK(t, "ulong");
}

int ilit(char *t)
{
    return isdigit(*t);
}

int star(char *t)
{
    return ISTOK(t, "*");
}