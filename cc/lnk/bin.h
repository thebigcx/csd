#pragma once

#include <stdint.h>

// Main header
struct bin_main
{
    uint64_t entry;  // Entry point
    uint32_t txtsz;  // Text section size
    uint32_t datsz;  // Data section size
    uint32_t txtrel; // Text section relocations
    uint32_t datrel; // Data section relocations
    uint32_t symtab; // Symbol table offset
    uint32_t strtab; // String table offset

    uint64_t base;   // Base address
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
    uint8_t  size;   // Size in bytes
    uint32_t sym;    // Symbol table index
    uint8_t  pcrel;  // Is RIP-relative
    int64_t  addend; // Addend
};