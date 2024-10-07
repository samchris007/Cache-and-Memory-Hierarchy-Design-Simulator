#ifndef MEMORY_H
#define MEMORY_H

#include <string>

class Memory
{
    public:  
        Memory* next;
        Memory* prev;
        int memoryPosition;

        virtual uint32_t ReadAddress(uint32_t address)
        {
            return address;
        }

        virtual void WriteAddress(uint32_t address){}
};

#endif