#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define NEW(T) (calloc(1, sizeof(T)))

extern FILE *g_in;  // Input file
extern FILE *g_out; // Output file
extern struct tok g_tok; // Current token

struct type
{
    int sign; // Signed/unsigned
    int size; // 0, 1, 2, 4, 8, etc.

    //int arrlen; // Array length (for computing size of entire array)
};

// AST types
enum
{
    A_BINOP,
    A_ILIT,
    A_OP,
    A_DECL,
    A_CMPD, // Compound statement
    A_IDENT
};

// Operator types
enum
{
    OP_ADD,
    OP_ASSIGN
};

// Astract Syntax Tree node
struct ast
{
    struct ast *left, *mid, *right;
    int type, op;

    uint64_t iv; // Integer value
    char *sv; // String value

    struct type vtype; // Variable type

    struct ast *next, *prev; // Next and previou statements (linked list for block of statements)
};

// Token types
enum
{
    T_SEMI,
    T_ILIT,
    T_PLUS,
    T_PUB,
    T_FN,
    T_LET,
    T_IDENT,
    T_COLON,
    T_U32,
    T_EQ,
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
struct tok *scan(); // Scan next token
int qualif(); // Current token qualifier?
struct tok expect(int t); // Expect a token

// expr.c
struct ast *expr(); // Parse expression

// stmt.c
struct ast *stmt(); // Parse statement
struct ast *cmpdstmt(); // Parse compound statement

// cg.c
int cg(struct ast *); // Generate code

// type.c
struct type type(); // Parse type