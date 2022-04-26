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

    uint8_t byte = 0;
    int opsz = 0, adrsz = 0, inst_set = 0;

    union modrm modrm = { 0 };

    while (!feof(g_in)) {
        // Read until valid opcode
        while (!valid_opcode(NXT(byte))) {
            switch (byte) {
                case 0x66: opsz = 1; continue;
                case 0x67: adrsz = 1; continue;
                case 0x0f: inst_set = 1; continue;
            }
        }

        if (byte == 0xff) break;
    
        struct optbl op = { 0 };
        optbl_from_opcode("/home/chris/opt/share/optbl.txt", inst_set ? 0x0f : 0, byte, &op);

        if (!(op.flag & OT_NOMODRM))
            modrm = (union modrm) { .bits = NXT(byte) };

        printf("%s %s, %s\n", op.mnem, reg_to_str(modrm.reg, 8), reg_to_str(modrm.rm, 8));
    }

    return 0;
}