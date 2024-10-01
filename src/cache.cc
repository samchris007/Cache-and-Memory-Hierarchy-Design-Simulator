#include "Cache.h"
#include <math.h>
#include <iostream>

using namespace std;

// bool Cache::addressExists(uint32_t address) {
//     for (int i = 0; i < sets-1; ++i) {
//         for (int j = 0; j < associativity-1; ++j) {
//             if (cacheArray[i][j].tag == address) {
//                 return true; // Element found
//             }
//         }
//     }
//     return false; // Element not found
// }

// bool Cache::addressExistsByIndex(uint32_t tagAddress, uint32_t index) {
//     for (int j = 0; j < associativity-1; ++j) {
//         if (cacheArray[index][j].tag == tagAddress) {
//             return true;
//         }
//     }
//     return false;
// }

// void Cache::readAddress(uint32_t address) {
//     uint32_t tag, indexBits;
//     splitBits(address, tag, indexBits);
//     if (addressExistsByIndex(tag, indexBits-1)){
//         printf("\n Address Exist");
//     }
//     else {
//         printf("\n Address Doesn't Exist");
//     }
// }