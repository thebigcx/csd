#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define ALEN(a) (sizeof(a) / sizeof(a[0]))

#define NEW(t) (calloc(1, sizeof(t)))

#define ISTOK(t, s) (!strcmp(t, s))

#define EXPECT(t)                                   \
{                                                   \
    char *e = token();                              \
    if (!ISTOK(e, t))                               \
        error("Expected '%s', got '%s'\n", t, e);   \
}

/* AST node types */
#define A_BINOP 0
#define A_ILIT  1
#define A_ID    2

struct ast /* AST node */
{
    int         type;
    struct ast *lhs, *rhs;
    char       *val;
};

void error(const char *msg, ...); /* Print error message and exit */

// lex.c
void lex_file(FILE*); /* Set working file */
char *token();        /* Get next token */
int oper(char*);      /* Is operator? */
int eof();            /* End of file? */
void tputbck(char*);  /* Putback token */
int stclass(char*);   /* Storage class? */
int type(char*);      /* Type? */
int ilit(char*);      /* Int literal? */

// expr.c
struct ast *expr(char*); /* Expression */

// stmt.c
void stmt(char*); /* Statement */
void decl(char*); /* Declaration/definition */

// cg.c
void cgfile(FILE*);   /* Set output file for code generator */
int cg(struct ast*);  /* Generate assembly for AST node */
void cgfndef(char*);  /* Generate function start */
void cgfnend();       /* Generate function end */
void cgscope(size_t); /* Enter stack frame */
void cgleave();       /* Leave stack frame */

// sym.c
typedef union type
{
    uint32_t bits;
    struct
    {
        uint32_t sgn : 1; // Signed
        uint32_t ptr : 3; // Pointer count
        uint32_t sz  : 4; // Size (1, 2, 4, 8)
    };
} type_t;

#define SC_AUTO  0 /* Auto     */
#define SC_STAT  1 /* Static   */
#define SC_EXTRN 2 /* Extern   */
#define SC_REG   3 /* Register */

struct sym /* Symbol */
{
    char        *name;
    int          class; // Storage class
    unsigned int off;   // Stack offset
    type_t       type;  // Type
    struct sym  *nxt;   // Next in linked-list
};

struct symtab /* Symbol table */
{
    struct sym    *syms;
    unsigned int   stckoff;
    struct symtab *parent;
};

void newscope();                      /* Start new scope */
void retscope();                      /* Return (end) scope */
struct sym *lookup(char*);            /* Lookup symbol */
unsigned int symstckoff(struct sym*); /* Compute stack offset */
void addsym(struct sym);              /* Add symbol */
unsigned int tysize(type_t*);         /* Compute type size */
size_t symsize();                     /* Get total symbol size */