// Intermediate code representation
#include "asm.h"

uint32_t psreg(const char *str)
{
    return (R_AX << 7) | OP_SB;
}