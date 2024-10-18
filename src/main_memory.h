#ifndef MAINMEMORY_H   // Include guard to prevent multiple inclusions
#define MAINMEMORY_H

#include "memory.h"

class MainMemory : public Memory
{
    public:
        MainMemory(int position)
        {
            memoryPosition = position;
            this -> next = nullptr;
            this -> prev = nullptr;
        }

        int MemoryTraffic = 0;
        
        uint32_t ReadAddress(uint32_t address) override
        {
            MemoryTraffic++;
            return address;
        }
        
        void WriteAddress(uint32_t address) override
        {
            MemoryTraffic++;
        }
};

#endif

