#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "Cache.h"
#include "sim.h"
#include "memory.h"
#include <bitset>
#include <iostream>
#include <math.h>
#include <sstream>
#include <iomanip>
using namespace std;

/*  "argc" holds the number of command-line arguments.
    "argv[]" holds the arguments themselves.

    Example:
    ./sim 32 8192 4 262144 8 3 10 gcc_trace.txt
    argc = 9
    argv[0] = "./sim"
    argv[1] = "32"
    argv[2] = "8192"
    ... and so on
*/

void CreateAndLinkNewMemory(Memory*& topMemory, Memory* newMemory)
{
   Memory* temp = topMemory;
   while (temp -> next != nullptr)
   {
      temp = temp->next;
   }
   newMemory -> prev = temp;
   temp -> next = newMemory;
}

void CreateAndLinkPrefetchUnit(Memory*& topMemory, CacheParameters& cacheParameters)
{
   Memory* temp = topMemory;
   while (temp -> next != nullptr)
   {
      Cache* cache = dynamic_cast<Cache*>(temp);
      if (temp->next->next == nullptr && cache)
      {
         PrefetchUnit* cachePrefetchUnit = new PrefetchUnit(cacheParameters.prefetchM, cacheParameters.prefetchN, cache->blockSize, cache->size, cache->associativity);
         // Link cache and main memory to the prefetch unit.
         temp -> prefetchUnit = cachePrefetchUnit;
         cachePrefetchUnit -> prev = temp;
         cachePrefetchUnit -> next = temp -> next;
      }
      temp = temp->next;
   }
}

void CreateHierarchicalCachesAndMainMemory(int cacheCount, Memory*& topMemory, CacheParameters* cacheParametersForCaches)
{
   if (cacheCount < 1)
   {
      return;
   }
   
   for (int memoryIndex = 0; memoryIndex <= cacheCount; memoryIndex++){
      if (memoryIndex == 0 || memoryIndex == cacheCount)
      { 
         if (topMemory == nullptr)
         {
            Cache* newCache = new Cache(cacheParametersForCaches[memoryIndex]);
            topMemory = newCache;
            continue;
         }

         int mainMemoryPosition = memoryIndex + 1;
         MainMemory* mainMemory = new MainMemory(mainMemoryPosition);
         CreateAndLinkNewMemory(topMemory, mainMemory);
         CreateAndLinkPrefetchUnit(topMemory, cacheParametersForCaches[memoryIndex-1]);
         continue;
      }

      Cache* newCache = new Cache(cacheParametersForCaches[memoryIndex]);
      CreateAndLinkNewMemory(topMemory, newCache);
   }
}

CacheElement* bubbleSort(CacheElement cacheArray[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (cacheArray[j].Counter > cacheArray[j + 1].Counter) {
                CacheElement temp = cacheArray[j];
                cacheArray[j] = cacheArray[j + 1];
                cacheArray[j + 1] = temp;
            }
        }
    }
    return cacheArray;
}

void ShowMemoryContents(Memory* topMemory)
{
   Memory* temp = topMemory;
   PrefetchUnit* prefetchUnit = dynamic_cast<PrefetchUnit*>(temp->prefetchUnit);
   PrefetchUnit* l2PrefetchUnit = nullptr;
   if (!prefetchUnit)
   {
      l2PrefetchUnit = dynamic_cast<PrefetchUnit*>(temp->next->prefetchUnit);
      prefetchUnit = l2PrefetchUnit;
   }
   while (temp -> next != nullptr)
   {
      Cache* cache = dynamic_cast<Cache*>(temp);
      if (!cache)
      {
         break;
      }
      if (cache->memoryPosition == 2)
      {
         printf("\n");
      }
      printf("===== L%u contents =====\n", cache->memoryPosition);
      for (int setIndex = 0; setIndex < cache->sets; ++setIndex) 
      {
         printf("set% *d:", 7, setIndex);
         CacheElement* sortedArray = bubbleSort(cache->CacheArray[setIndex], cache->associativity);
         for (int assocIndex = 0; assocIndex < cache->associativity; ++assocIndex)
         {
            printf("% *x", 11, sortedArray[assocIndex].Tag);
            if (sortedArray[assocIndex].DirtyBit == 1)
            {
               printf(" D   ");
               continue;
            }
            printf("     ");
         }
         printf("\n");
      }
      temp = temp->next;
   }
   if (prefetchUnit && prefetchUnit->streamBuffersCount != 0)
   {
      printf("\n===== Stream Buffer(s) contents =====\n");
      int tempCount = prefetchUnit->streamBuffersCount*prefetchUnit->streamBuffersCount;
      int elementsPrinted = 0;
      for (int i = 0; i < tempCount; i++)
      {
         int modValue = i % prefetchUnit->streamBuffersCount;
         if (prefetchUnit->streamBuffers[modValue]->Counter == elementsPrinted)
         {
            elementsPrinted++;
            while(!prefetchUnit->streamBuffers[modValue]->Stream.empty())
            {
               printf("%x ", prefetchUnit->streamBuffers[modValue]->Stream.front().TagAndIndex);
               prefetchUnit->streamBuffers[modValue]->Stream.pop();
            }
            printf("\n");
         }
         if (elementsPrinted == prefetchUnit->streamBuffersCount)
         {
            break;
         }
      }
   }
}

void ShowOutputsOfMemoryHierarchy(Memory* topMemory)
{
   Memory* temp = topMemory;
   PrefetchUnit* l2PrefetchUnit = nullptr;
   MainMemory* mainMemoryInL2 = nullptr;
   Cache* L1 = dynamic_cast<Cache*>(temp);
   Cache* L2 = dynamic_cast<Cache*>(temp -> next);
   PrefetchUnit* l1PrefetchUnit = dynamic_cast<PrefetchUnit*>(L1->prefetchUnit);
   MainMemory* mainMemoryInL1 = dynamic_cast<MainMemory*>(temp->next);
   if (L2)
   {
      mainMemoryInL2 = dynamic_cast<MainMemory*>(temp->next->next);
      l2PrefetchUnit = dynamic_cast<PrefetchUnit*>(L2->prefetchUnit);
   }

   printf("\n===== Measurements =====\n");
   printf("a. L1 reads: %20u\n", L1->Read);
   printf("b. L1 Read misses: %14u\n", L1->ReadMiss);
   printf("c. L1 writes: %19u\n", L1->Write);
   printf("d. L1 write misses: %13u\n", L1->WriteMiss);
   printf("e. L1 miss rate: %16.4f\n", L1->GetMissRate());
   printf("f. L1 writebacks: %15u\n", L1->WriteBack);
   printf("g. L1 prefetches: %15u\n", l1PrefetchUnit ? l1PrefetchUnit->Prefetches : 0);
   printf("h. L2 reads (demand): %11u\n", L2 ? L2->Read : 0);
   printf("i. L2 read misses (demand): %5u\n",L2 ? L2->ReadMiss : 0);
   printf("j. L2 reads (prefetch): %9u\n", 0);
   printf("k. L2 read misses (prefetch): %3u\n", 0);
   printf("l. L2 writes: %19u\n", L2 ? L2->Write : 0);
   printf("m. L2 write misses: %13u\n", L2 ? L2->WriteMiss : 0);
   printf("n. L2 miss rate: %16.4f\n", L2 ? L2->GetMissRate() : 0);
   printf("o. L2 writebacks: %15u\n", L2? L2->WriteBack : 0);
   printf("p. L2 prefetches: %15u\n", l2PrefetchUnit ? l2PrefetchUnit->Prefetches : 0);
   printf("q. memory traffic: %14u \n", mainMemoryInL1 ? mainMemoryInL1->MemoryTraffic : mainMemoryInL2->MemoryTraffic);
}

int main (int argc, char *argv[]) {
   FILE *fp;			// File pointer.
   char *trace_file;		// This variable holds the trace file name.
   cache_params_t params;	// Look at the sim.h header file for the definition of struct cache_params_t.
   char rw;			// This variable holds the request's type (read or write) obtained from the trace.
   uint32_t addr;		// This variable holds the request's address obtained from the trace.
				// The header file <inttypes.h> above defines signed and unsigned integers of various sizes in a machine-agnostic way.  "uint32_t" is an unsigned integer of 32 bits.

   // Exit with an error if the number of command-line arguments is incorrect.
   // argv = { "/sim.exe", "32", "8192", "4", "262144", "8", "3","10","/gcc_trace.txt"};
   // argv[0] = strdup("C:\\Users\\samch\\OneDrive\\Documents\\NCSU\\563\\Cache Project\\Cache-and-Memory-Hierarchy-Simulator\\sim.cc");
   // argv[1] = strdup("16");
   // argv[2] = strdup("1024");
   // argv[3] = strdup("1");
   // argv[4] = strdup("8192");
   // argv[5] = strdup("4");
   // argv[6] = strdup("3");
   // argv[7] = strdup("4");
   // argv[8] = strdup("C:\\Users\\samch\\OneDrive\\Documents\\NCSU\\563\\Cache Project\\Cache-and-Memory-Hierarchy-Simulator\\benchmarks\\gcc_trace.txt");
   // argc = 9;

   if (argc != 9) {
      printf("Error: Expected 8 command-line arguments but was provided %d.\n", (argc - 1));
      exit(EXIT_FAILURE);
   }
    
   // "atoi()" (included by <stdlib.h>) converts a string (char *) to an integer (int).
   params.BLOCKSIZE = (uint32_t) atoi(argv[1]);
   params.L1_SIZE   = (uint32_t) atoi(argv[2]);
   params.L1_ASSOC  = (uint32_t) atoi(argv[3]);
   params.L2_SIZE   = (uint32_t) atoi(argv[4]);
   params.L2_ASSOC  = (uint32_t) atoi(argv[5]);
   params.PREF_N    = (uint32_t) atoi(argv[6]);
   params.PREF_M    = (uint32_t) atoi(argv[7]);
   trace_file       = argv[8];

   int cacheCount = 0;
   if (params.L1_SIZE != 0)
   {
      cacheCount++;
      if (params.L2_SIZE != 0)
      {
         cacheCount++;
      }
   }

   CacheParameters* cacheParametersForCaches = new CacheParameters[cacheCount];
   bool prefetchExistsInL1 = cacheCount == 1 && params.PREF_M != 0 || params.PREF_N != 0 ? true : false;
   cacheParametersForCaches[0] = { 1, params.BLOCKSIZE ,params.L1_ASSOC, params.L1_SIZE, params.PREF_M, params.PREF_N, prefetchExistsInL1 };

   if (params.L2_ASSOC != 0 && params.L2_SIZE != 0)
   {
      bool prefetchExistsInL2 = cacheCount == 2 && params.PREF_M != 0 || params.PREF_N != 0 ? true : false;
      cacheParametersForCaches[1] = { 2, params.BLOCKSIZE ,params.L2_ASSOC, params.L2_SIZE, params.PREF_M, params.PREF_N, prefetchExistsInL2 };
   }
   
   Memory* topMemory = nullptr;
   CreateHierarchicalCachesAndMainMemory(cacheCount, topMemory, cacheParametersForCaches);
   
   // Open the trace file for reading.
   fp = fopen(trace_file, "r");
   if (fp == (FILE *) NULL) {
      // Exit with an error if file open failed.
      printf("Error: Unable to open file %s\n", trace_file);
      exit(EXIT_FAILURE);
   }
    
   // Print simulator configuration.
   printf("===== Simulator configuration =====\n");
   printf("BLOCKSIZE:  %u\n", params.BLOCKSIZE);
   printf("L1_SIZE:    %u\n", params.L1_SIZE);
   printf("L1_ASSOC:   %u\n", params.L1_ASSOC);
   printf("L2_SIZE:    %u\n", params.L2_SIZE);
   printf("L2_ASSOC:   %u\n", params.L2_ASSOC);
   printf("PREF_N:     %u\n", params.PREF_N);
   printf("PREF_M:     %u\n", params.PREF_M);
   printf("trace_file: %s\n", trace_file);
   printf("\n");

   while (fscanf(fp, "%c %x\n", &rw, &addr) == 2)
   {
      if (rw == 'r')
      {
         topMemory->ReadAddress(addr);
      }
      else if (rw == 'w')
      {
         topMemory->WriteAddress(addr);
      }
      else
      {
         printf("Error: Unknown request type %c.\n", rw);
	      exit(EXIT_FAILURE);
      }
   }
   ShowMemoryContents(topMemory);
   ShowOutputsOfMemoryHierarchy(topMemory);
   return(0);
}
