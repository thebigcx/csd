#pragma once

#include <stdint.h>
#include <stdio.h>

#define NEW(T) (calloc(1, sizeof(T)))

extern FILE *g_in;  // Input file
extern FILE *g_out; // Output file

// AST types
enum
{
    A_BINOP,
    A_ILIT,
    A_OP
};

// Operator types
enum
{
    OP_ADD
};

// Astract Syntax Tree node
struct ast
{
    struct ast *left, *mid, *right;
    int op;

    uint64_t iv;
};

// Token types
enum
{
    T_ILIT,
    T_PLUS,
    T_EOF
};

// Token
struct tok
{
    int type;
    char *sv; // String
    uint64_t iv; // Integer
};

// scan.c
struct tok scan(); // Scan next token

// expr.c
struct ast *expr(); // Parse expression

// stmt.c
struct ast *stmt(); // Parse statement

// cg.c
int cg(struct ast *); // Generate code