// disas: disassembles a machine code file in the binary format

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <lib/optbl.h>
#include <lnk/bin.h>

FILE *g_in = NULL;

// File metadata
char          *strtab = NULL;
struct symbol *symtab = NULL;
unsigned int   symcnt = 0;

int      binary_input    = 0;
size_t   text_size       = 0;
size_t   base_address    = 0;
uint64_t section_offset  = 0;
uint64_t program_counter = 0;

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

// SIB byte
union sib 
{
    struct // Little-endian (reverse order)
    {
        uint8_t base : 3;
        uint8_t idx  : 3;
        uint8_t scl  : 2;
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

// Read file metadata (if not raw binary)
static void read_metadata()
{
    if (!binary_input) {
        struct bin_main main;
        fread(&main, sizeof(struct bin_main), 1, g_in);
    
        text_size = main.txtsz;

        // Read string table and symbol table
        fseek(g_in, 0, SEEK_END);
        size_t file_size = ftell(g_in);

        symcnt = (main.strtab - main.symtab) / sizeof(struct symbol);
        symtab = malloc(main.strtab - main.symtab);

        fseek(g_in, main.symtab, SEEK_SET);
        fread(symtab, sizeof(struct symbol), symcnt, g_in);

        strtab = malloc(file_size - main.strtab);
        fread(strtab, file_size - main.strtab, 1, g_in);

        fseek(g_in, sizeof(struct bin_main), SEEK_SET);

    } else {
        fseek(g_in, 0, SEEK_END);
        text_size = ftell(g_in);
        fseek(g_in, 0, SEEK_SET);
    }
}

// Get a symbol given its value
static struct symbol *getsymbol(uint64_t val)
{
    for (unsigned int i = 0; i < symcnt; i++) {
        if (symtab[i].value == val)
            return &symtab[i];
    }

    return NULL;
}

int valid_opcode(uint8_t byte)
{
    return byte != 0x66 && byte != 0x67 && byte != 0x0f
        && (byte < 0x40 || byte > 0x4f);
}

uint64_t read_immediate(size_t size)
{
    // Read in immediate
    uint64_t imm = 0;
    uint8_t byte = 0;

    for (uint8_t j = 0; j < size; j++)
        imm |= NXT(byte) << (j * 8);

    return imm;
}

// Read in an immediate operand and print it
void do_immediate(struct optbl *optbl, op_t *op)
{
    uint64_t imm = read_immediate(op->size);

    if (optbl->flag & OT_REL) {
        int64_t rel = (int64_t)imm + (ftell(g_in) - section_offset) + base_address;
        
        printf(" 0x%lx", rel);

        // Check if relative to label TODO: closest symbol if none match perfectly
        struct symbol *sym = getsymbol(rel);
        if (sym)
            printf(" <%s>", strtab + sym->name);
    } else
        printf(" 0x%lx", imm);
}

void do_register(union modrm *modrm, struct optbl *optbl, op_t *op)
{
    // If no ModR/M, use the specific register in the optbl
    uint8_t reg = optbl->flag & OT_NOMODRM ? op->reg
                : op->type == (OTT_REG | OTT_MEM) ? modrm->rm : modrm->reg;
    printf(" %s", reg_to_str(reg, op->size));
}

void do_memory(union modrm *modrm, struct optbl *optbl, op_t *op)
{
    printf(" u%d", op->size * 8);

    // Easy case: no SIB byte
    if ((modrm->rm & 0b111) != 0b100) {
        printf(" [%s", reg_to_str(modrm->rm, 8)); // TODO: address-size override

        // Displacements
        if (modrm->mod == 1) printf(" + 0x%lx", read_immediate(1));
        if (modrm->mod == 2) printf(" + 0x%lx", read_immediate(4));

        printf("]");
        return;
    }

    // Hard case: SIB byte
    // TODO: special cases sp/bp

    uint8_t byte;
    union sib sib = { .bits = NXT(byte) };

    printf(" [%s + (%s * %d)", reg_to_str(sib.base, 8), reg_to_str(sib.idx, 8), 1 << sib.scl);

    // Displacements
    if (modrm->mod == 1) printf(" + 0x%lx", read_immediate(1));
    if (modrm->mod == 2) printf(" + 0x%lx", read_immediate(4));

    printf("]");
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("usage: disas <inputfile> -b\n");
        exit(-1);
    }

    char *input = NULL;

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-b"))
            binary_input = 1;
        else if (!strcmp(argv[i], "-a"))
            base_address = strtol(argv[++i], NULL, 16);
        else
            input = argv[i];
    }

    if (!input) {
        printf("usage: disas <inputfile> -b");
        exit(-1);
    }

    if (!(g_in = fopen(input, "r"))) {
        printf("disas: %s: %s\n", input, strerror(errno));
        exit(-1);
    }

    atexit(cleanup);

    read_metadata();

    printf("Disassembly of text section:\n");

    program_counter = base_address;
    section_offset  = ftell(g_in);

    while (ftell(g_in) - section_offset < text_size) {
        // Start of instruction
        uint64_t strt = ftell(g_in);

        // Check if there exists a label at the current RIP
        struct symbol *sym = getsymbol(program_counter);
        if (sym)
            printf("0x%lx <%s>:\n", program_counter, strtab + sym->name);

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
        
        struct optbl op = { 0 };
        optbl_from_opcode("/home/chris/opt/share/optbl.txt", inst_set ? 0x0f : 0, po, opsz, rex & 0b1000, -1, &op);

        if (!(op.flag & OT_NOMODRM))
            modrm.bits = NXT(byte);

        // For a +r opcode, remove 3 bits and set ModR/M.reg
        if (op.flag & OT_REGPO) {
            modrm.reg = po & 0b111;
            po &= ~0b111;
        }

        // Seach again using ModR/M.reg (for instructions with /0, /2, etc.)
        optbl_from_opcode("/home/chris/opt/share/optbl.txt", inst_set ? 0x0f : 0, po, opsz, rex & 0b1000, modrm.reg, &op);

        printf("  %lx:\t%s", program_counter, op.mnem);

        // Foreach operand
        for (int i = 0; i < 3; i++) {
            
            // Format instruction nicely
            if (op.ops[i].type)
                if (!i) printf("\t");
                else    printf(",");

            if (op.ops[i].type == OTT_IMM) do_immediate(&op, &op.ops[i]);
            else {
                if (modrm.mod != 3 && op.ops[i].type & OTT_MEM)
                    do_memory(&modrm, &op, &op.ops[i]);
                else if (op.ops[i].type & OTT_REG)
                    do_register(&modrm, &op, &op.ops[i]);
            }
        }

        printf("\n");

        program_counter += ftell(g_in) - strt;
    }

    return 0;
}