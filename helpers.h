#ifndef HELPERS_H   // Include guard to prevent multiple inclusions
#define HELPERS_H

#include "memory.h"

class Helpers
{
    public:
        static void splitBits(const uint32_t address, int blockSize, int sets, uint32_t& tag, uint32_t& indexBits, uint32_t& tagAndIndex) 
        {
            uint32_t offsetBitsCount = log2(blockSize);
            uint32_t indexBitsCount = log2(sets);
            uint32_t mask = (1 << indexBitsCount) - 1;

            tagAndIndex = address >> offsetBitsCount;
            indexBits = tagAndIndex & mask;
            tag = tagAndIndex >> indexBitsCount;
        }
};

#endif