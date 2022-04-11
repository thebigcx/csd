#include "cc.h"

#include <string.h>

static struct symtab *s_symtab = NULL;

void setscope(struct symtab *symtab)
{
    s_symtab = symtab;
}

struct symtab *getscope()
{
    return s_symtab;
}

// Lookup within symbol table and recurse to parents
static struct sym *_lookup(struct symtab *symtab, const char *name)
{
    if (!symtab) return NULL;

    // Traverse linked list
    for (struct sym *sym = symtab->head; sym; sym = sym->next)
        if (!strcmp(name, sym->name)) return sym;

    return _lookup(symtab->parent, name); // Not found, try parents
}

struct sym *lookup(const char *name)
{
    return _lookup(s_symtab, name);
}

struct sym *addsym(const char *name, struct type type, int flags)
{
    // Find last symbol in table
    struct sym **last = &s_symtab->head;
    for (; *last; last = &(*last)->next);

    // Create new symbol
    *last = NEW(struct sym);

    (*last)->name = strdup(name);
    (*last)->type = type;
    (*last)->flags = flags;

    // Stack flag, give it offset if required
    if (s_symtab->parent)
    {
        (*last)->flags |= S_STCK;
        (*last)->stckoff = -(s_symtab->stcksz += typesize(&type));
    }

    return *last;
}