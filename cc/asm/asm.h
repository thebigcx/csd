#pragma once

#include <stdio.h>
#include <stdint.h>

enum TOKEN
{
    T_IDENT,
    T_EOF,
    TOKCNT
};

// Token
struct tok
{
    int type;
    char *sv; // String value
    uint64_t iv; // Integer value
};

struct mem
{
    uint64_t disp;
    int dispsz;

    int base, idx;
    uint8_t scale;

    int used;
};

// Parsed instruction operand
struct op
{ 
    uint32_t type; // Operand type and size

    // Value
    union
    {
        int reg;
        uint64_t imm;
        struct mem mem;
    };
};

// Parsed assembly code
struct code
{
    char *mnem; // Mnemonic
    struct op op1, op2, op3; // Operands

    struct op *mem;

    // These fields used to pass info from opcode searcher to assembler
    uint64_t imm;
};

// Low 4 bits of register, high bits are size
#define R_AX  0b0000
#define R_CX  0b0001
#define R_DX  0b0010
#define R_BX  0b0011
#define R_SP  0b0100
#define R_BP  0b0101
#define R_SI  0b0110
#define R_DI  0b0111
#define R_R8  0b1000
#define R_R9  0b1001
#define R_R10 0b1010
#define R_R11 0b1011
#define R_R12 0b1100
#define R_R13 0b1101
#define R_R14 0b1110
#define R_R15 0b1111

#define R_NUL (-1)

#define OR_REGR (1 << 3) // /r

// Types
#define OP_TR (1 << 0) // Reg
#define OP_TM (1 << 1) // Mem
#define OP_TI (1 << 2) // Imm

#define OP_TMASK (0b111) // Type bit mask
#define OP_TYPE(op) (op & OP_TMASK)

// Operands Sizes
#define OP_SB (1 << 3) // Byte
#define OP_SW (1 << 4) // Word
#define OP_SD (1 << 5) // Doubleword
#define OP_SQ (1 << 6) // Quadword

#define OP_SMASK (0b1111 << 3) // Size bit mask
#define OP_SIZE(op) (op & OP_SMASK)

#define OP_RMASK (0b11111111 << 7) // Register bit mask
#define OP_REG(op) ((op & OP_RMASK) >> 7)

// Opcode specification (information parsed from special file)
struct opcode
{
    const char *mnem; // Mnemonic

    uint8_t rex; // REX prefix (e.g. denotes %spl reg, 64-bit operands)

    uint8_t of; // Prefix 0f
    uint8_t po; // Primary Opcode
    uint8_t r;  // Register/Opcode field

    uint32_t op1, op2, op3; // Operands

    uint8_t imm; // Immediate size in bytes (0 if unused)
};

extern FILE *g_in;
extern FILE *g_out;
extern struct tok g_tok;

// code.c
struct code pscode(); // Parse code
uint32_t psreg(const char *str); // Parse register

// asm.c
void assem(struct code *code, struct opcode *opcode); // Assemble code

// opcode.c
struct opcode matchop(struct code *code); // Match opcode from parsed code