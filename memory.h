#ifndef MEMORY_H
#define MEMORY_H

#include <string>

class Memory
{
    public:  
        Memory* next;
        Memory* prev;
        Memory* prefetchUnit = nullptr;
        int memoryPosition;

        virtual uint32_t ReadAddress(uint32_t address)
        {
            return address;
        }

        virtual void WriteAddress(uint32_t address){}

        virtual bool TryGetIndexFromStreamBuffer(uint32_t address, int &streamBufferIndex)
        {
            streamBufferIndex = 0;
            return false;
        }

        virtual void SetValuesToStreamBuffer(uint32_t address, int streamBufferIndex){}

        virtual void  updateElementsInTheLRUStreamBuffer(uint32_t address){}
};

#endif