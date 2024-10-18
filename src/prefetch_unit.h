#ifndef PREFETCHUNIT_H   // Include guard to prevent multiple inclusions
#define PREFETCHUNIT_H

#include <string>
#include <math.h>
#include <queue>
#include "memory.h"
#include "main_memory.h"
#include "sim.h"
#include "helpers.h"
#include "stream_buffer.h"

class PrefetchUnit : public Memory
{
    public:
        StreamBuffer** streamBuffers;
        unsigned long Prefetches = 0;
        int streamBuffersCount = 0;
        int streamBufferElementsCount = 0;

    public:
        PrefetchUnit(const int mCount, const int nCount, int cacheBlockSize, int cacheSize, int associativity)
        {
            streamBufferElementsCount = mCount;
            streamBuffersCount = nCount;
            blockSize = cacheBlockSize;
            sets = cacheSize / (blockSize * associativity);
            streamBuffers = new StreamBuffer*[streamBuffersCount];

            for (int i = 0; i < nCount; ++i) {
                streamBuffers[i] = new StreamBuffer(mCount, blockSize, sets, Prefetches);
                streamBuffers[i]->Counter = i;
            }

            this->next = nullptr;
            this->prev = nullptr;
        }

        bool TryGetIndexFromStreamBuffer(uint32_t address, int &streamBufferIndex) override
        {
            uint32_t tag, index, tagAndIndex;
            Helpers::splitBits(address, blockSize, sets, tag, index, tagAndIndex);
            int streamBufferMaxCounter = streamBuffersCount-1;
            bool isHit = false;
            for (int i = 0; i < streamBuffersCount; ++i)
            {
                if (streamBuffers[i]->TagAndIndexExistsInStream(tagAndIndex, streamBuffers[i]->Stream))
                {
                    if (streamBuffers[i]->Counter <= streamBufferMaxCounter)
                    {
                        streamBufferMaxCounter = streamBuffers[i]->Counter;
                        streamBufferIndex = i;
                    }
                    isHit = true;
                }
            }
            return isHit;
        }

        void SetValuesToStreamBuffer(uint32_t address, int streamBufferIndex) override
        {
            streamBuffers[streamBufferIndex]->SetValues(address, next);
            incrementCounterValuesToStreamBuffers(streamBufferIndex);
        }

        void updateElementsInTheLRUStreamBuffer(uint32_t address) override
        {
            int lruElementCounter = streamBuffersCount-1;
            for (int i = 0; i < streamBuffersCount; ++i) 
            {
                if (streamBuffers[i]->Counter == lruElementCounter)
                {
                    SetValuesToStreamBuffer(address, i);
                    return;
                }
            }
        }

        void incrementCounterValuesToStreamBuffers(int indexOfReadStreamBuffer)
        {
            for (int i = 0; i < streamBuffersCount; ++i)
            {
                if (streamBuffers[i]->Counter < streamBuffers[indexOfReadStreamBuffer]->Counter)
                {
                    streamBuffers[i]->Counter++;
                }
             }
            streamBuffers[indexOfReadStreamBuffer]->Counter = 0;
        }

    private:
        int blockSize;
        int sets;
};


#endif  // End of include guard