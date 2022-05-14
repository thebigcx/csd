#include "mcc.h"

static struct symtab *s_curtab = NULL;

void newscope()
{
    struct symtab *par = s_curtab;

    s_curtab = NEW(struct symtab);
    s_curtab->parent = par;
}

void retscope()
{
    struct symtab *par = s_curtab->parent;
    free(s_curtab);
    s_curtab = par;
}

// Lookup implementation
static struct sym *_lookup(char *name, struct symtab *tab)
{
    if (!tab) return NULL;

    for (struct sym *s = tab->syms; s; s = s->nxt)
        if (!strcmp(s->name, name)) return s;
    
    return _lookup(name, tab->parent);
}

struct sym *lookup(char *name)
{
    return _lookup(name, s_curtab);    
}

unsigned int symstckoff(struct sym *sym)
{
    if (s_curtab == sym->tab) return sym->off;

    unsigned int off = sym->off;
    for (struct symtab *s = s_curtab->parent; s->parent; s = s->parent) {
        off -= s->stckoff + 8;
        if (sym->tab == s) break;
    }

    return off;
}

void addsym(struct sym s)
{
    // Get last symbol
    struct sym **last = &s_curtab->syms;
    while (*last)
        last = &(*last)->nxt;

    if (s.class == SC_AUTO) {
        s.off = s_curtab->stckoff;
        s_curtab->stckoff += tysize(&s.type);
    }

    addsymoff(s);
}

void addsymoff(struct sym s)
{
    // Get last symbol
    struct sym **last = &s_curtab->syms;
    while (*last)
        last = &(*last)->nxt;

    *last  = NEW(struct sym);
    **last = s;

    (*last)->tab = s_curtab;
}

unsigned int tysize(type_t *t)
{
    if (t->ptr) return 8;

    return t->sz;
}

struct symtab *curtab()
{
    return s_curtab;
}

type_t gettype(char *t)
{
    type_t type = { 0 };

    // Signed?
    if (*t != 'u') type.sgn = 1;
    else t++;

    // Size?
         if (!strcmp(t, "char")) type.sz = 1;
    else if (!strcmp(t, "shrt")) type.sz = 2;
    else if (!strcmp(t, "int"))  type.sz = 4;
    else if (!strcmp(t, "long")) type.sz = 8;

    // Parse '*' for pointers
    while (star(t = token()))
        type.ptr++;

    tputbck(t);
    return type;
}