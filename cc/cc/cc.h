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
    int func; // Function type
    int ptr;  // Number of pointers

    //int arrlen; // Array length (for computing size of entire array)

    struct type *param, *next; // Linked list of function parameters
};

// AST types
enum
{
    A_BINOP,
    A_UNARY,
    A_ILIT,
    A_OP,
    A_DECL,
    A_CMPD, // Compound statement
    A_IF,
    A_IDENT
};

// Operator types
enum
{
    OP_ADD,
    OP_ASSIGN,
    OP_DEREF,
    OP_LT,
};

// Symbol
struct sym
{
    const char *name;
    struct type type;
    int global; // Global/local
    int stckoff; // Offset to stack pointer
    
    struct sym *next; // Next symbol in linked list
};

struct symtab
{
    struct sym *head; // Head of linked-list
    int stcksz; // Current stack size
    struct symtab *parent; // Parent symbol table
};

// Astract Syntax Tree node
struct ast
{
    struct ast *left, *mid, *right;
    int type, op;

    uint64_t iv; // Integer value
    char *sv; // String value

    struct type vtype; // Variable type

    struct symtab symtab; // Symbol table

    struct ast *next, *prev; // Next and previous statements (linked list for block of statements)
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
    T_LPAREN,
    T_RPAREN,
    T_COMMA,
    T_STAR,
    T_LBRACE,
    T_RBRACE,
    T_IF,
    T_EOF,
    T_LT,
    TOKCNT
};

extern const char *tokstrs[TOKCNT];
#define TOKSTR(t) (tokstrs[t])

// Token
struct tok
{
    int type;
    char *sv; // String
    uint64_t iv; // Integer
};

// scan.c
struct tok *scan(); // Scan next token
int qualif(); // Is current token qualifier?
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
unsigned int typesize(struct type *t);

// sym.c
void setscope(struct symtab *symtab); // Set symbol table scope
struct symtab *getscope(); // Get symbol table scope

struct sym *lookup(const char *name); // Look up symbol in current scope
struct sym *addsym(const char *name, struct type type); // Add symbol to symbol table