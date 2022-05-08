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
    for (struct sym *s = tab->syms; s; s = s->nxt)
        if (!strcmp(s->name, name)) return s;
    
    return NULL;
}

struct sym *lookup(char *name)
{
    return _lookup(name, s_curtab);    
}

unsigned int symstckoff(struct sym *sym)
{
    unsigned int off = sym->off;
    for (struct symtab *s = s_curtab->parent; s; s = s->parent)
        off += s->stckoff + 8;

    return off;
}

void addsym(struct sym s)
{
    // Get last symbol
    struct sym **last = &s_curtab->syms;
    while (*last && (*last)->nxt)
        last = &(*last)->nxt;

    if (s.class == SC_AUTO) {
        s.off = s_curtab->stckoff += tysize(&s.type);
    }

    *last  = NEW(struct sym);
    **last = s;
}

unsigned int tysize(type_t *t)
{
    if (t->ptr) return 8;

    return t->sz;
}