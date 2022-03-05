#pragma once

#include <stdint.h>

// Astract Syntax Tree node
struct ast
{

};

// Token types
enum
{
    T_EOF
};

// Token
struct tok
{
    int type;
    char *s; // String
    uint64_t i; // Integer
};

// scan.c
struct tok scan(); // Scan next token

// expr.c
struct ast *expr(); // Parse expression

// stmt.c
struct ast *stmt(); // Parse statement