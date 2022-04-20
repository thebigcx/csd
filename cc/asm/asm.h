#pragma once

#include <stdio.h>
#include <stdint.h>

enum TOKEN
{
    T_IDENT,
    T_NL, // Newline
    T_EOF,
    T_ILIT,
    T_COMMA,
    T_LBRACK,
    T_RBRACK,
    T_LPAREN,
    T_RPAREN,
    T_STAR,
    T_PLUS,
    T_COLON,
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

    int used, size;
};

// Parsed instruction operand
struct op
{ 
    uint32_t type; // Operand type and size

    char *lbl; // Label for immediate operand or displacement

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
    struct op op[3]; // Operands

    // Easy access - don't need to search every time
    struct op *mem, *rm, *imm;
};

// Label
struct label
{
    char *name;
    uint64_t val;
    uint8_t flags;

    unsigned int idx; // For the binary output
};

// Forward reference to resolve later, add as relocation
struct forward
{
    char    *lbl;
    uint64_t pc;
    int      size;
    uint8_t  flags;

    struct sect *sect; // Section

    int line; // Line number (for debugging)
};

#define SE_TEXT 0
#define SE_DATA 1
#define SE_BSS  2

// Section
struct sect
{
    int      type;
    uint64_t offset;
    size_t   size;
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
#define OR_REGP (1 << 4) // +r
#define OR_UNUSED (1 << 5)  // ModR/M.reg field unused

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

    uint8_t osovr; // Operand-size override
    uint8_t asovr; // Address-size override

    uint8_t rex; // REX prefix (e.g. denotes %spl reg, 64-bit operands)

    uint8_t of; // Prefix 0f
    uint8_t po; // Primary Opcode
    uint8_t r;  // Register/Opcode field

    uint32_t op[3]; // Operands

    uint8_t imm; // Immediate size in bytes (0 if unused)
    uint8_t rel; // Relative size in bytes (0 if unused)
};

extern char *g_infile; // Input file name
extern FILE *g_in;
extern FILE *g_out;
extern struct tok g_tok;
extern int g_mode; // Real mode (2), Protected Mode (4), Long Mode (8)
extern struct forward *g_forwards;
extern unsigned int g_forwardcnt;
extern unsigned int g_line;
extern uint32_t g_bss; // BSS size

void error(const char *msg, ...);
void general_error(const char *msg, ...); // No specific line number or file

// code.c
struct code pscode(); // Parse code
uint32_t psreg(const char *str); // Parse register
void dofile(); // Assemble file

// asm.c
void assem(struct code *code, struct opcode *opcode); // Assemble code
void addlabel(char *name, uint64_t pc);
void addextern(char *name); // Add an external symbol
void forwardref(char *name, int size, int pcrel);
struct label *resolvelbl(char *name);

struct label *getlbls();
unsigned int labelcnt();

void resolve_forwardrefs();

void emitb(uint8_t b); // Emit byte
void emitw(uint16_t w); // Emit word
void emitd(uint32_t d); // Emit doubleword
void emitq(uint64_t q); // Emit quadword
void emitv(uint64_t v, int size); // Emit bytes (size)
void emit(uint64_t v); // Emit minimum size

unsigned int getpc(); // Get program counter

// opcode.c
struct opcode matchop(struct code *code); // Match opcode from parsed code

// scan.c
struct tok *scan();
void expect(int tok);

// bin.c
void binheader(); // Write binary file header
void binfini();   // Finalize
void startsect(char *name); // Start new section
void setsect(int type);     // Switch to section
struct sect *getsect();