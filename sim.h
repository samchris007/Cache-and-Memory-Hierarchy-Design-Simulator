#ifndef SIM_CACHE_H
#define SIM_CACHE_H

typedef 
struct {
   uint32_t BLOCKSIZE;
   uint32_t L1_SIZE;
   uint32_t L1_ASSOC;
   uint32_t L2_SIZE;
   uint32_t L2_ASSOC;
   uint32_t PREF_N;
   uint32_t PREF_M;
} cache_params_t;

struct CacheParameters
{
    int CachePosition;
    uint32_t BlockSize;
    uint32_t Associativity;
    uint32_t Size;
    uint32_t prefetchM;
    uint32_t prefetchN;
    bool PrefetchUnitExists;
};

// Put additional data structures here as per your requirement.

#endif
