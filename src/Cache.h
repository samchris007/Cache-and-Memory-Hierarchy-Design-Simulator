#ifndef CACHE_H   // Include guard to prevent multiple inclusions
#define CACHE_H

#include <string>

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
        uint32_t Address = 0;
        uint32_t IndexBits = 0;
        int ValidBit = 0;
        int DirtyBit = 0;
        int Counter;

        void SetValues(uint32_t address, uint32_t tag, uint32_t index, bool isWriteOperation)
        {
            Tag = tag;
            IndexBits = index;
            Address = address;
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

        virtual uint32_t ReadAddress(uint32_t address)
        {
            return address;
        }

        virtual void WriteAddress(uint32_t address){}
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
        
        void WriteAddress(uint32_t address) override
        {
            MemoryTraffic++;
        }
};

// Class declaration in the header file
class Cache : public Memory {
    private:
        int blockSize;
        int size;

    public:
        int sets;
        int associativity;
        CacheElement **CacheArray;
        unsigned long Read = 0;
        unsigned long Write = 0;
        unsigned long ReadMiss = 0;
        unsigned long WriteMiss = 0;
        unsigned long ReadHit = 0;
        unsigned long WriteHit = 0;
        unsigned long WriteBack = 0;

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

        uint32_t ReadAddress(uint32_t address) override
        {
            uint32_t tag = 0;
            uint32_t index = 0;
            int assocIndex = 0;
            
            Read++;
            splitBits(address, tag, index);
            if (addressExistsByIndex(tag, index, assocIndex))
            {
                ReadHit++;
                incrementCounterValuesToCacheElements(index, assocIndex);
                CacheArray[index][assocIndex].Counter = 0;
                return address;
            }

            ReadMiss++;
            writeBackToNextMemoryIfLRUBlockIsDirty(address, tag, index);
            uint32_t addressValue = next -> ReadAddress(address);
            updateTagInTheLRUBlock(address, tag, index, false);
            return addressValue;
        }

        void WriteAddress(uint32_t address) override
        {
            uint32_t tag = 0;
            uint32_t index = 0;
            int assocIndex = 0;
            
            Write++;
            splitBits(address, tag, index);
            if (addressExistsByIndex(tag, index, assocIndex))
            {
                WriteHit++;
                incrementCounterValuesToCacheElements(index, assocIndex);
                CacheArray[index][assocIndex].Counter = 0;
                CacheArray[index][assocIndex].DirtyBit = 1;
                return;
            }

            WriteMiss++;
            writeBackToNextMemoryIfLRUBlockIsDirty(address, tag, index);
            uint32_t addressValue = next -> ReadAddress(address);
            updateTagInTheLRUBlock(address, tag, index, true);
            return;
        }

    private:
        bool addressExistsByIndex(uint32_t tagAddress, int setIndex, int& assocIndex)
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

        bool TryGetAssociativityIndexofLRUBlock(uint32_t tagAddress, int setIndex, int& assocIndex)
        {
            for (assocIndex = 0; assocIndex < associativity; ++assocIndex)
            {
                if (CacheArray[setIndex][assocIndex].ValidBit == 1 &&
                    CacheArray[setIndex][assocIndex].Counter == (associativity-1))
                {
                    return true;
                }
            }
            return false;
        }

        void updateTagInTheLRUBlock(uint32_t address, uint32_t tag, uint32_t indexBits, bool isWriteOperation)
        {
            int lruElementCounter = associativity-1;
            for (int setIndex = 0; setIndex < sets; ++setIndex) 
            {
                for (int assocIndex = 0; assocIndex < associativity; ++assocIndex)
                {
                    if (setIndex == indexBits && CacheArray[setIndex][assocIndex].Counter == lruElementCounter)
                    {   
                        storeAddressInCacheSet(setIndex, assocIndex, address, tag, isWriteOperation);
                        return;
                    }
                }
            }
        }

        void incrementCounterValuesToCacheElements(int index, int assocIndexOfReadValue)
        {
            for (int assocIndex = 0; assocIndex < associativity; ++assocIndex)
            {
                if (CacheArray[index][assocIndex].Counter < CacheArray[index][assocIndexOfReadValue].Counter)
                {
                    CacheArray[index][assocIndex].Counter++;
                }
            }
        }

        void storeAddressInCacheSet(int index, int assocIndexOfReadValue, uint32_t address, uint32_t tag, bool isWriteOperation)
        {
            incrementCounterValuesToCacheElements(index, assocIndexOfReadValue);
            CacheElement newCacheElement = CacheElement();
            newCacheElement.SetValues(address, tag, index, isWriteOperation);
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

        void writeBackToNextMemoryIfLRUBlockIsDirty(uint32_t address, uint32_t tag, int index)
        {
            int assocIndex = 0;
            if (TryGetAssociativityIndexofLRUBlock(tag, index, assocIndex)
                && CacheArray[index][assocIndex].DirtyBit == 1)
            {
                writeBackToNextMemory(CacheArray[index][assocIndex].Address, index, assocIndex);
            }
        }

        void writeBackToNextMemory(uint32_t address, int index, int assocIndex)
        {
            WriteBack++;
            CacheArray[index][assocIndex].DirtyBit = 0;
            next -> WriteAddress(address);
        }
};

#endif  // End of include guard