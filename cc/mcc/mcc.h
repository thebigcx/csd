#pragma once

#include <stdio.h>
#include <stdlib.h>

#define ALEN(a) (sizeof(a) / sizeof(a[0]))

#define NEW(t) (calloc(1, sizeof(t)))

#define A_BINOP 0
#define A_ILIT  1

// Abstract Syntax Tree node
struct ast
{
    int         type;
    struct ast *lhs, *rhs;
    char       *val;
};

// lex.c
void lex_file(FILE*); /* Set working file */
char *token();        /* Get next token */
int oper(char *t);    /* Is operator? */

// expr.c
struct ast *expr(char*); /* Expression */

// cg.c
void cgfile(FILE*);  /* Set output file for code generator */
int cg(struct ast*); /* Generate assembly for AST node */