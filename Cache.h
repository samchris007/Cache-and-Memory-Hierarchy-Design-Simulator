#ifndef CACHE_H   // Include guard to prevent multiple inclusions
#define CACHE_H

#include <string>  // Required for using the string type

struct CacheParameters
{
    uint32_t CachePosition;
    uint32_t BlockSize;
    uint32_t Associativity;
    uint32_t Size;
};

class Memory
{
public:  
    Memory* next;
    Memory* prev;
    int memoryPosition;
};

class MainMemory : public Memory
{
public:
    MainMemory(int position)
    {
        memoryPosition = position;
        this->next = nullptr;
        this->prev = nullptr;
    }
};

// Class declaration in the header file
class Cache : public Memory {

public:
    Cache(const CacheParameters& cacheParameters)
    {
        memoryPosition = cacheParameters.CachePosition;
        blockSize = cacheParameters.BlockSize;
        associativity = cacheParameters.Associativity;
        size = cacheParameters.Size;
        sets = size / (blockSize * associativity);

        cacheArray = new CacheElement*[sets];
        for (int i = 0; i < sets; ++i) 
        {
            cacheArray[i] = new CacheElement[associativity];
        }
        this->next = nullptr;
        this->prev = nullptr;
    }

public:
    class CacheElement 
    {  
        public:
            uint32_t tag;
            uint32_t indexBits;
            uint32_t validBit;
            uint32_t dirtyBit;
            uint32_t lruCounter;
    };CacheElement **cacheArray;

private:
    uint32_t blockSize;
    uint32_t associativity;
    uint32_t size;
    uint32_t sets;

public:
    bool addressExists(uint32_t address);
    bool Cache::addressExistsByIndex(uint32_t tagAddress, uint32_t index) {
    for (int j = 0; j < associativity; ++j) {
        if (cacheArray[index][j].tag == tagAddress) {
            return true;
        }
    }
    return false;
}

void Cache::readAddress(uint32_t address) {
    uint32_t tag, indexBits;
    splitBits(address, tag, indexBits);
    if (addressExistsByIndex(tag, indexBits-1)){
        printf("\n Address Exist");
    }
    else {
        printf("\n Address Doesn't Exist");
    }
}

    //Write to CACHE
    void writeToAddress(unsigned long);

    void splitBits(const uint32_t address, uint32_t& tag, uint32_t& indexBits) 
    {
        uint32_t offsetBitsCount = log2(blockSize);
        uint32_t indexBitsCount = log2(sets);
        uint32_t tagAndIndex = address >> offsetBitsCount;
        uint32_t mask = (1 << indexBitsCount) - 1;

        indexBits = tagAndIndex & mask;
        tag = tagAndIndex >> indexBitsCount;
    }
};

#endif  // End of include guard