#pragma once

#include <stdio.h>
#include <lnk/bin.h>

// Symbol
struct sym
{
    char *name;
    uint64_t val;
    uint8_t flags;

    int index;   // Index in object file
    int fileidx; // File index (for offset stuff when combining objects)
};

// Section types
#define SE_TEXT 1
#define SE_DATA 2
#define SE_BSS  3

// Section
struct sect
{
    int type;
    int fileidx;
    uint64_t base;
};

extern FILE        *g_in[16];
extern unsigned int g_incnt;
extern FILE        *g_out;
extern uint64_t     g_base;
extern struct sym  *g_symtab;
extern unsigned int g_symcnt;
extern struct sect *g_sects;
extern unsigned int g_sectcnt;

void do_binary(); // Do binary file
void do_normal(); // Do normal output
void write_section(int f, int type); // Write a section
void *readfile(FILE *f); // Read file contents

struct sym resolve(struct sym *undf);

void error(const char *msg, ...); // Fatal error (doesn't return)