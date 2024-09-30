#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "sim.h"
#include "Cache.h"
#include "Constants.h"
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

void CreateAndLinkNewMemory(int memoryIndex, Memory*& topMemory, Memory* newMemory)
{
   Memory* temp = topMemory;
   while (temp -> next != nullptr)
   {
        temp = temp->next;
   }
   newMemory -> prev = temp;
   temp -> next = newMemory;
}

void CreateHierarchicalCachesAndMainMemory(Memory*& topMemory, CacheParameters* cacheParametersForCaches)
{
   if (CACHE_COUNT < 1)
   {
      return;
   }
   
   for (int memoryIndex = 0; memoryIndex <= CACHE_COUNT; memoryIndex++){
      if (memoryIndex == 0 || memoryIndex == CACHE_COUNT)
      { 
         if (topMemory == nullptr)
         {
            Cache* newCache = new Cache(cacheParametersForCaches[memoryIndex]);
            topMemory = newCache;
            continue;
         }

         MainMemory* mainMemory = new MainMemory(memoryIndex);
         CreateAndLinkNewMemory(memoryIndex, topMemory, mainMemory);
         continue;
      }

      Cache* newCache = new Cache(cacheParametersForCaches[memoryIndex]);
      CreateAndLinkNewMemory(memoryIndex, topMemory, newCache);
   }
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
   argv[0] = strdup("C:\\Users\\samch\\OneDrive\\Documents\\NCSU\\563\\Cache Project\\Cache-and-Memory-Hierarchy-Simulator\\sim.cc");
   argv[1] = strdup("16");
   argv[2] = strdup("1024");
   argv[3] = strdup("1");
   argv[4] = strdup("0");
   argv[5] = strdup("0");
   argv[6] = strdup("0");
   argv[7] = strdup("0");
   argv[8] = strdup("C:\\Users\\samch\\OneDrive\\Documents\\NCSU\\563\\Cache Project\\Cache-and-Memory-Hierarchy-Simulator\\gcc_trace.txt");
   argc = 9;

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

   CacheParameters* cacheParametersForCaches = new CacheParameters[CACHE_COUNT];
   cacheParametersForCaches[0] = { 0, params.BLOCKSIZE ,params.L1_ASSOC, params.L1_SIZE };

   if (params.L2_ASSOC != 0 && params.L2_SIZE != 0)
   {
      cacheParametersForCaches[1] = { 1, params.BLOCKSIZE ,params.L2_ASSOC, params.L2_ASSOC };
   }
   
   Memory* topMemory = nullptr;
   CreateHierarchicalCachesAndMainMemory(topMemory, cacheParametersForCaches);
   
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

   // Read requests from the trace file and echo them back.
   while (fscanf(fp, "%c %x\n", &rw, &addr) == 2) {	// Stay in the loop if fscanf() successfully parsed two tokens as specified.
      if (rw == 'r'){
         topMemory->ReadAddress(addr);
      }
      else if (rw == 'w'){
         // printf("w %x\n", addr);
         // uint32_t tag, indexBits;
         // L1.splitBits(addr, tag, indexBits);
      }
      else {
         printf("Error: Unknown request type %c.\n", rw);
	 exit(EXIT_FAILURE);
      }
      
    }
   
    Memory* temp = topMemory;
    while (temp->next != nullptr) {
        temp = temp->next;
    }

    return(0);
}
