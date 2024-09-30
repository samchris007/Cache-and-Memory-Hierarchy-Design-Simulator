#ifndef CACHE_H   // Include guard to prevent multiple inclusions
#define CACHE_H

#include <string>  // Required for using the string type

struct CacheParameters
{
    int CachePosition;
    uint32_t BlockSize;
    uint32_t Associativity;
    uint32_t Size;
};

class CacheElement 
{  
    public:
        uint32_t Tag = 0;
        uint32_t IndexBits = 0;
        int ValidBit = 0;
        int DirtyBit = 0;
        int Counter;

        void SetValues(uint32_t tag, uint32_t index, bool isWriteOperation)
        {
            Tag = tag;
            IndexBits = index;
            ValidBit = 1;
            
            if (isWriteOperation)
            {
                DirtyBit = 1;
            }
        }
};

class Memory
{
    public:  
        Memory* next;
        Memory* prev;
        int memoryPosition;

        virtual uint32_t ReadAddress(uint32_t address){
            return address;
        }
};

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

            CacheArray = new CacheElement*[sets];
            for (int i = 0; i < sets; ++i) 
            {
                CacheArray[i] = new CacheElement[associativity];
            }

            for (int setIndex = 0; setIndex < sets; ++setIndex) 
            {
                for (int assocIndex = 0; assocIndex < associativity; ++assocIndex)
                {
                    CacheArray[setIndex][assocIndex].Counter = assocIndex;
                }
            }

            this->next = nullptr;
            this->prev = nullptr;
        }

    public:
        CacheElement **CacheArray;
        unsigned long Read = 0;
        unsigned long ReadMiss = 0;
        unsigned long ReadHit = 0;

    private:
        int blockSize;
        int associativity;
        int size;
        int sets;

    public:
        bool AddressExistsByIndex(uint32_t tagAddress, int setIndex, int& assocIndex)
        {
            for (assocIndex = 0; assocIndex < associativity; ++assocIndex)
            {
                if (CacheArray[setIndex][assocIndex].ValidBit == 1 &&
                    CacheArray[setIndex][assocIndex].Tag == tagAddress)
                {
                    return true;
                }
            }
            return false;
        }

        uint32_t ReadAddress(uint32_t address) override
        {
            uint32_t tag = 0;
            uint32_t index = 0;
            int assocIndex = 0;
            
            Read++;
            splitBits(address, tag, index);
            if (AddressExistsByIndex(tag, index, assocIndex))
            {
                this->ReadHit+=1;
                IncrementCounterValuesToCacheElements(index, assocIndex);
                return address;
            }

            this->ReadMiss+=1;
            uint32_t addressValue = next->ReadAddress(address);
            UpdateTagInCacheWhenReadMiss(tag, index);
            return addressValue;
        }

        void UpdateTagInCacheWhenReadMiss(uint32_t tag, uint32_t indexBits)
        {
            int lruElementCounter = associativity-1;
            for (int setIndex = 0; setIndex < sets; ++setIndex) 
            {
                for (int assocIndex = 0; assocIndex < associativity; ++assocIndex)
                {
                    if (setIndex == indexBits && CacheArray[setIndex][assocIndex].Counter == lruElementCounter)
                    {   
                        storeAddressInCacheSet(setIndex, assocIndex, tag);
                        return;
                    }
                }
            }
        }

        void IncrementCounterValuesToCacheElements(int index, int assocIndexOfReadValue)
        {
            for (int assocIndex = 0; assocIndex < associativity; ++assocIndex)
            {
                if (CacheArray[index][assocIndex].Counter < CacheArray[index][assocIndexOfReadValue].Counter)
                {
                    CacheArray[index][assocIndex].Counter++;
                }
            }
        }

        void storeAddressInCacheSet(int index, int assocIndexOfReadValue, uint32_t tag)
        {
            IncrementCounterValuesToCacheElements(index, assocIndexOfReadValue);
            CacheElement newCacheElement = CacheElement();
            newCacheElement.SetValues(tag, index, false);
            newCacheElement.Counter = 0;
            CacheArray[index][assocIndexOfReadValue] = newCacheElement;
        }

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