#include "mcc.h"

#include <string.h>
#include <ctype.h>

FILE *s_f = NULL;

// List of symbol tokens
static char *s_toks[] = {
    "{"
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