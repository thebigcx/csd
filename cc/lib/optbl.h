// optbl: opcode table parsing library for my listing format

#pragma once

#include <stdint.h>

#define OT_OPSZ    (1 << 0) // Address-size override
#define OT_ADRSZ   (1 << 1) // Operand-size override
#define OT_REGPO   (1 << 2) // '+R': Add reg to primary opcode
#define OT_REL     (1 << 3) // Relative operand
#define OT_INPRE   (1 << 4) // Instruction prefix 0x0f
#define OT_REXW    (1 << 5) // REX.W prefix
#define OT_NOMODRM (1 << 6) // ModR/M unused
#define OT_REGR    (1 << 7) // /r ModR/M

#define OTT_REG 1 // Register
#define OTT_MEM 2 // Memory
#define OTT_IMM 3 // Immediate

typedef struct
{
    uint32_t type : 2; // OTT_
    uint32_t size : 4;
    uint32_t reg  : 4;
} op_t;

struct optbl
{
    char mnem[8];
    uint8_t po;

    uint32_t flag;
    uint32_t modr;

    op_t ops[3];
};

int optbl_foreach(const char *file, void (*fn)(struct optbl*)); /* Loop over each line of optbl */
int optbl_from_opcode(const char *file, uint8_t pre, uint8_t opcode,
                      struct optbl *op); /* Find optbl from opcode */