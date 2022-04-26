// disas: disassembles a machine code file in the binary format

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <lib/optbl.h>

FILE *g_in = NULL;

#define NXT(b) (b = fgetc(g_in))

// ModR/M byte
union modrm
{
    struct // Little-endian (reverse order)
    {
        uint8_t rm  : 3;
        uint8_t reg : 3;
        uint8_t mod : 2;
    };
    uint8_t bits;
};

struct regstr
{
    const char *str;
    uint8_t size;
    uint8_t val;
};

#define REGSTR(s, sz, v) { .str = s, .size = sz, .val = v }

static struct regstr regstrs[] = {
    REGSTR("al", 1, 0),
    REGSTR("cl", 1, 1),
    REGSTR("dl", 1, 2),
    REGSTR("bl", 1, 3),
    
    REGSTR("ax", 2, 0),
    REGSTR("cx", 2, 1),
    REGSTR("dx", 2, 2),
    REGSTR("bx", 2, 3),

    REGSTR("eax", 4, 0),
    REGSTR("ecx", 4, 1),
    REGSTR("edx", 4, 2),
    REGSTR("ebx", 4, 3),

    REGSTR("rax", 8, 0),
    REGSTR("rcx", 8, 1),
    REGSTR("rdx", 8, 2),
    REGSTR("rbx", 8, 3),
};

static const char *reg_to_str(uint8_t reg, uint8_t size)
{
    for (size_t i = 0; i < sizeof(regstrs) / sizeof(regstrs[0]); i++) {
        if (regstrs[i].val == reg && regstrs[i].size == size)
            return regstrs[i].str;
    }

    return "(null)";
}

static void cleanup()
{
    fclose(g_in);
}

int valid_opcode(uint8_t byte)
{
    return byte != 0x66 && byte != 0x67 && byte != 0x0f
        && (byte < 0x40 || byte > 0x4f);
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("usage: disas <inputfile>\n");
        exit(-1);
    }

    if (!(g_in = fopen(argv[1], "r"))) {
        printf("disas: %s: %s\n", argv[1], strerror(errno));
        exit(-1);
    }

    atexit(cleanup);

    while (!feof(g_in)) {
        union modrm modrm = { 0 };
        uint8_t byte = 0;
        uint8_t rex = 0;
        int opsz = 0, adrsz = 0, inst_set = 0;

        uint8_t po = 0;

        // Read until valid opcode
        while (!valid_opcode(NXT(byte))) {
            switch (byte) {
                case 0x66: opsz = 1; continue;
                case 0x67: adrsz = 1; continue;
                case 0x0f: inst_set = 1; continue;
            }

            // REX
            rex = byte;
        }

        if (byte == 0xff) break;

        po = byte;
        
        size_t size = opsz ? 2
                    : rex & 0b1000 ? 8
                    : 4;

        struct optbl op = { 0 };
        optbl_from_opcode("/home/chris/opt/share/optbl.txt", inst_set ? 0x0f : 0, po, size, &op);

        if (!(op.flag & OT_NOMODRM))
            modrm.bits = NXT(byte);

        // For a +r opcode, remove 3 bits and set ModR/M.reg
        if (op.flag & OT_REGPO) {
            modrm.reg = po & 0b111;
            po &= ~0b111;
        }

        //optbl_from_opcode("/home/chris/opt/share/optbl.txt", inst_set ? 0x0f : 0, po, &op);

        printf("\t%s", op.mnem);

        // Foreach operand
        for (int i = 0; i < 3; i++) {

            if (op.ops[i].type == OTT_IMM) {
                // Read in immediate
                uint64_t imm = 0;

                for (uint8_t j = 0; j < op.ops[i].size; j++) {
                    imm |= NXT(byte) << (j * 8);
                }
                
                printf(" 0x%x", imm);
            }

            if (op.ops[i].type == OTT_REG) {
                
                // If no ModR/M, use the specific register in the optbl
                uint8_t reg = op.flag & OT_NOMODRM ? op.ops[i].reg : modrm.reg;
                printf(" %s", reg_to_str(modrm.reg, op.ops[i].size));
            }
                
            if (op.ops[i].type & OTT_MEM)
                printf(" %s", reg_to_str(modrm.rm, op.ops[i].size));
        }

        printf("\n");
    }

    return 0;
}