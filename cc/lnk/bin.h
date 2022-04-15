#pragma once

#include <stdint.h>

struct bin_main
{
    uint64_t entry; // Entry point
    uint32_t txtsz; // Text section size
    uint32_t datsz; // Data section size
};