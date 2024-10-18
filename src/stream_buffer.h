#ifndef STREAMBUFFER_H   // Include guard to prevent multiple inclusions
#define STREAMBUFFER_H

#include <queue>
#include "memory.h"
#include "main_memory.h"
#include "sim.h"
#include "helpers.h"

class StreamBuffer
{
    public:
        int ValidBit = 0;
        int Counter;
        unsigned long& Prefetches;
        queue<StreamBufferElement> Stream;

        StreamBuffer(int count, int cacheBlockSize, int streamBufferSets, unsigned long& prefetches) : Prefetches(prefetches)
        {
            elementsCount = count;
            Prefetches = prefetches;
            blockSize = cacheBlockSize;
            sets = streamBufferSets;
            indexBitsCount = log2(sets);
        }

        void SetValues(uint32_t address, Memory* mainMemory)
        {
            uint32_t tag, index, tagAndIndex;
            Helpers::splitBits(address, blockSize, sets, tag, index, tagAndIndex);
            queue<StreamBufferElement> tempStream = Stream;
            ClearValues();

            for (int m = 1; m <= elementsCount; ++m)
            {
                uint32_t newTagAndIndex = tagAndIndex + m;
                StreamBufferElement element = StreamBufferElement();
                element.TagAndIndex = newTagAndIndex;
                Stream.push(element);
            }

            if (TagAndIndexExistsInStream(tagAndIndex, tempStream))
            {
                int elementIndex = GetStreamIndexofAddress(tagAndIndex, tempStream);
                int elementsRemoved = elementIndex+1;
                Prefetches = Prefetches + elementsRemoved;
                for (int i = 0; i < elementsRemoved; i++)
                {
                    mainMemory->ReadAddress(address);
                }
            }
            else
            {
                Prefetches = Prefetches + elementsCount;
                for (int i = 0; i < elementsCount; i++)
                {
                    mainMemory->ReadAddress(address);
                }
            }
        }

        bool TagAndIndexExistsInStream(uint32_t tagAndIndex, queue<StreamBufferElement> stream)
        {
            queue<StreamBufferElement> tempQueue = stream;

            while (!tempQueue.empty()) 
            {
                if (tempQueue.front().TagAndIndex == tagAndIndex)
                {
                    return true;
                }
                tempQueue.pop();
            }
            return false;
        }

        int GetStreamIndexofAddress(uint32_t tagAndIndex, queue<StreamBufferElement> stream)
        {
            int elementIndex = 0;
            queue<StreamBufferElement> tempQueue = stream;

            while (!tempQueue.empty()) 
            {
                if (tempQueue.front().TagAndIndex == tagAndIndex)
                {
                    return elementIndex;
                }
                tempQueue.pop();
                elementIndex++;
            }
            return elementIndex;
        }

    private:
        int elementsCount = 0;
        int blockSize = 0;
        int sets = 0;
        int indexBitsCount;

        void ClearValues()
        {
            while (!Stream.empty())
            {
                Stream.pop();
            }
        }
};


#endif  // End of include guard