#ifndef CACHE_H   // Include guard to prevent multiple inclusions
#define CACHE_H

#include <string>
#include <math.h>
#include <queue>
#include "memory.h"
#include "main_memory.h"
#include "sim.h"
#include "helpers.h"

using namespace std;

class CacheElement 
{  
    public:
        uint32_t Tag = 0;
        uint32_t Address = 0;
        uint32_t IndexBits = 0;
        uint32_t TagAndIndex = 0;
        int ValidBit = 0;
        int DirtyBit = 0;
        int Counter;

        void SetValues(uint32_t address, uint32_t tag, uint32_t index, uint32_t tagAndIndex, bool isWriteOperation)
        {
            Tag = tag;
            IndexBits = index;
            Address = address;
            TagAndIndex = tagAndIndex;
            ValidBit = 1;
            
            if (isWriteOperation)
            {
                DirtyBit = 1;
            }
        }
};

class Cache : public Memory {
    public:
        int blockSize;
        int size;
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
            uint32_t tagAndIndex = 0;
            int assocIndex = 0;
            
            Read++;
            Helpers::splitBits(address, blockSize, sets, tag, index, tagAndIndex);
            if (addressExistsByIndex(tag, index, assocIndex))
            {
                ReadHit++;
                incrementCounterValuesToCacheElements(index, assocIndex);
                CacheArray[index][assocIndex].Counter = 0;
                if (prefetchUnit)
                {
                    TryGetAddressFromPrefetcher(address);
                }
                return address;
            }

            writeBackToNextMemoryIfLRUBlockIsDirty(address, tag, index);
            if (TryGetAddressFromPrefetcher(address))
            {
                updateTagInTheLRUBlock(address, tag, index, false);
                return address;
            }
            
            if (prefetchUnit)
            {
                prefetchUnit->updateElementsInTheLRUStreamBuffer(address);
            }

            ReadMiss++;
            uint32_t addressValue = next -> ReadAddress(address);
            updateTagInTheLRUBlock(address, tag, index, false);
            return addressValue;
        }

        void WriteAddress(uint32_t address) override
        {
            uint32_t tag = 0;
            uint32_t index = 0;
            uint32_t tagAndIndex = 0;
            int assocIndex = 0;
            
            Write++;
            Helpers::splitBits(address, blockSize, sets, tag, index, tagAndIndex);
            if (addressExistsByIndex(tag, index, assocIndex))
            {  
                int streamBufferIndex;

                WriteHit++;
                incrementCounterValuesToCacheElements(index, assocIndex);
                CacheArray[index][assocIndex].Counter = 0;
                CacheArray[index][assocIndex].DirtyBit = 1;
                if (prefetchUnit)
                {
                    TryGetAddressFromPrefetcher(address);
                }
                return;
            }

            writeBackToNextMemoryIfLRUBlockIsDirty(address, tag, index);
            if (TryGetAddressFromPrefetcher(address))
            {
                updateTagInTheLRUBlock(address, tag, index, true);
                return;
            }
            
            if (prefetchUnit)
            {
                prefetchUnit->updateElementsInTheLRUStreamBuffer(address);
            }
            WriteMiss++;
            next -> ReadAddress(address);
            updateTagInTheLRUBlock(address, tag, index, true);
        }

        float GetMissRate()
        {
            if (memoryPosition == 1)
            {
                return (float)(ReadMiss + WriteMiss) / (Read + Write);
            }
            return (float)ReadMiss / Read;
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

        bool TryGetAddressFromPrefetcher(uint32_t address)
        {
            int streamBufferIndex = 0;
            if (prefetchUnit && prefetchUnit->TryGetIndexFromStreamBuffer(address, streamBufferIndex))
            {
                prefetchUnit->SetValuesToStreamBuffer(address, streamBufferIndex);
                return true;
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
            for (uint32_t setIndex = 0; setIndex < sets; ++setIndex) 
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
            uint32_t indexBits, tagAndIndex;
            incrementCounterValuesToCacheElements(index, assocIndexOfReadValue);
            Helpers::splitBits(address, blockSize, sets, tag, indexBits, tagAndIndex);
            CacheElement newCacheElement = CacheElement();
            newCacheElement.SetValues(address, tag, index, tagAndIndex, isWriteOperation);
            newCacheElement.Counter = 0;
            CacheArray[index][assocIndexOfReadValue] = newCacheElement;
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