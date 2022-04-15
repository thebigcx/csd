#pragma once

#include <stdint.h>

// Main header
struct bin_main
{
    uint64_t entry;  // Entry point
    uint32_t txtsz;  // Text section size
    uint32_t datsz;  // Data section size
    uint32_t symtab; // Symbol table
    uint32_t strtab; // String table
};

// Symbol structure
struct symbol
{
    uint32_t name;
    uint64_t value;
};

// Relocation structure
struct rel
{
    uint64_t addr;   // Byte offset into file
    uint8_t size;    // Size in bytes
    uint32_t sym;    // Symbol table index
    uint8_t pcrel;   // Is RIP-relative
    uint64_t addend; // Addend
};