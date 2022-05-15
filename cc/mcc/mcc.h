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

#define VT_INT (type_t) { .sz = 4, .sgn = 1 }

// Type
typedef union type
{
    uint32_t bits;
    struct
    {
        uint32_t sgn : 1; // Signed
        uint32_t ptr : 3; // Pointer count
        uint32_t sz  : 4; // Size (1, 2, 4, 8)
        uint32_t fn  : 1; // Function?
    };
} type_t;

/* AST node types */
#define A_BINOP 0
#define A_ILIT  1
#define A_ID    2
#define A_DEREF 3
#define A_CALL  4
#define A_IF    5
#define A_WHILE 6
#define A_UNARY 7
#define A_ADDR  8
#define A_STRLT 9

struct ast /* AST node */
{
    int         type;
    struct ast *lhs, *rhs;
    char       *val;
    type_t      vt; // Type
    int         l1, l2; // Labels

    struct ast *nxt, *prv; /* Linked list of function parameters */
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
int star(char*);      /* '*' */
char *strlit(char*);    /* String literal? */

// expr.c
struct ast *expr(char*); /* Expression */

// stmt.c
void stmt(char*); /* Statement */
void decl(char*); /* Declaration/definition */

struct sym;

// cg.c
void cgfile(FILE*);         /* Set output file for code generator */
int cg(struct ast*);        /* Generate assembly for AST node */
void cgfndef(char*, int);   /* Generate function start */
void cgfnend();             /* Generate function end */
void cgscope(size_t);       /* Enter stack frame */
void cgleave();             /* Leave stack frame */
void cgvardef(struct sym*); /* Define variable */
void cgretrn(struct ast*);  /* Generate retrn */
void cgif(struct ast*);
void cgelse(struct ast*);
void cgifend(struct ast*);
void cgwhile(struct ast*);
void cgwhileend(struct ast*);
void cgdiscard(int);        /* Discard the result */
void cgbyte(char);
void cgstrs();              /* Generate string literals */

// sym.c
#define SC_AUTO  0 /* Auto     */
#define SC_STAT  1 /* Static   */
#define SC_EXTRN 2 /* Extern   */
#define SC_REG   3 /* Register */
#define SC_PUB   4 /* Public   */

struct sym /* Symbol */
{
    char          *name;
    int            class; // Storage class
    unsigned int   off;   // Stack offset or register index
    type_t         type;  // Type
    struct sym    *nxt;   // Next in linked-list
    struct symtab *tab;   // Table
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
void addsymoff(struct sym);           /* Add symbol at stack offset */
unsigned int tysize(type_t*);         /* Compute type size */
struct symtab *curtab();              /* Get symbol table */
type_t gettype(char*);                /* Parse type */