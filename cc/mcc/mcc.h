#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ALEN(a) (sizeof(a) / sizeof(a[0]))

#define NEW(t) (calloc(1, sizeof(t)))

#define A_BINOP 0
#define A_ILIT  1

#define ISTOK(t, s) (!strcmp(t, s))

#define EXPECT(t)                                   \
{                                                   \
    char *e = token();                              \
    if (!ISTOK(e, t))                               \
        error("Expected '%s', got '%s'\n", t, e);   \
}

// Abstract Syntax Tree node
struct ast
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

// expr.c
struct ast *expr(char*); /* Expression */

// stmt.c
void stmt(char*); /* Statement */
void decl(char*); /* Declaration/definition */

// cg.c
void cgfile(FILE*);  /* Set output file for code generator */
int cg(struct ast*); /* Generate assembly for AST node */
void cgfndef(char*); /* Generate function start */
void cgfnend();      /* Generate function end */