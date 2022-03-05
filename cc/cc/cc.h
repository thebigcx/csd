#pragma once

#include <stdint.h>

// Astract Syntax Tree node
struct ast
{

};

// Token
struct tok
{
    int type;
    char *s; // String
    uint64_t i; // Integer
};

struct tok *scan(); // Scan next token