#pragma once

#include <stdio.h>

#define ALEN(a) (sizeof(a) / sizeof(a[0]))

// lex.c
void lex_file(FILE *f); /* Set working file */
char *token(); /* Get next token */