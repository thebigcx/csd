#include "asm.h"

#include <lnk/bin.h>
#include <string.h>

static struct bin_main s_main = { 0 };
static uint64_t s_sectoff = 0;
static uint64_t s_datoff = 0;

void binheader()
{
    fwrite(&s_main, sizeof(struct bin_main), 1, g_out);
    setsect("text");
}

void setsect(char *name)
{
    if (!strcmp(name, "text"))
        s_sectoff = sizeof(struct bin_main);
    else if (!strcmp(name, "data"))
    {
        if (s_datoff)
            s_sectoff = s_datoff;
        else
            s_sectoff = s_datoff = ftell(g_out);
    }
}

void binfini()
{
    fseek(g_out, 0, SEEK_END);

    s_main.txtsz = s_datoff - sizeof(struct bin_main);
    s_main.datsz = ftell(g_out) - s_datoff;

    fseek(g_out, 0, SEEK_SET);
    fwrite(&s_main, sizeof(struct bin_main), 1, g_out);
}

uint64_t getsect()
{
    return s_sectoff;
}

char *getsectname()
{
    if (s_sectoff == sizeof(struct bin_main)) return "text";
    else return "data";
}