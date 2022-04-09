#include "cc.h"

#include <string.h>

static struct sym **s_symtab = NULL;

void setscope(struct sym **symtab)
{
    s_symtab = symtab;
}

struct sym **getscope()
{
    return s_symtab;
}

struct sym *lookup(const char *name)
{
    // Traverse linked list
    for (struct sym *sym = *s_symtab; sym; sym = sym->next)
        if (!strcmp(name, sym->name)) return sym;

    return NULL; // Not found
}

struct sym *addsym(const char *name, struct type type)
{
    // Find last symbol in table
    struct sym **last = s_symtab;
    for (; *last; last = &(*last)->next);

    // Create new symbol
    *last = NEW(struct sym);

    (*last)->name = strdup(name);
    (*last)->type = type;
    (*last)->stckoff = (*s_symtab)->stcksz += typesize(&type);

    return *last;
}