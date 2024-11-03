# Cache and Memory Hierarchy Design Simulator

## Overview

- This project is designed as a simulator of a generic cache module that can be used at any level of a memory hierarchy. For example, this cache module can be instantiated as an L1 cache, L2 cache, L3 cache, and so on. 

- The simulator also includes a prefetch unit as an extension within the CACHE. It maintains a clean abstraction, allowing one or more instances of CACHE to interact within an overall memory hierarchy, where each CACHE may include a prefetch unit.

## Prerequisites
Ensure you have the following installed:
- [Programming language complier, e.g., VC++]

## Usage
To run the project:
1. Navigate to the project directory:
    ```bash
    cd your-project-folder
    ```
2. Run the main script using the below mentioned command line inputs.

### Command Line Inputs

Simulator accepts exactly 8 command-line arguments in the following order:
```bash
    sim <BLOCKSIZE> <L1_SIZE> <L1_ASSOC> <L2_SIZE> <L2_ASSOC> <PREF_N> <PREF_M> <trace_file>
```
- `BLOCKSIZE`: Positive integer. Block size in bytes. (Same block size for all 
caches in the memory hierarchy.)
- `L1_SIZE`: Positive integer. L1 cache size in bytes.
- L1_ASSOC: Positive integer. L1 set-associativity (1 is direct-mapped, 
`L1_SIZE`/`BLOCKSIZE` is fully-associative).
12
- `L2_SIZE`: Positive integer. L2 cache size in bytes. `L2_SIZE` = 0 signifies that there is no L2 cache.
- `L2_ASSOC`: Positive integer. L2 set-associativity (1 is direct-mapped, 
`L2_SIZE`/`BLOCKSIZE` is fully-associative).
- PREF_N: Positive integer. Number of Stream Buffers in the L1 prefetch unit
(if there is no L2) or the L2 prefetch unit (if there is an L2). PREF_N = 0 
disables the prefetch unit.
- `PREF_M`: Positive integer. Number of memory blocks in each Stream Buffer 
in the L1 prefetch unit (if there is no L2) or the L2 prefetch unit (if there is an 
L2).
- `trace_file`: Character string. Full name of trace file including any 
extensions.

- Example: 8KB 4-way set-associative L1 cache with 32B block size, 256KB 8-
way set-associative L2 cache with 32B block size, L2 prefetch unit has 3 stream 
buffers with 10 blocks each, gcc trace:
sim 32 8192 4 262144 8 3 10 gcc_trace.txt.

### Command Line Outputs

- **Number of L1 reads**
- **Number of L1 read misses**, excluding L1 read misses that hit in the stream buffers if the L1 prefetch unit is enabled
- **Number of L1 writes**
- **Number of L1 write misses**, excluding L1 write misses that hit in the stream buffers if the L1 prefetch unit is enabled
- **L1 miss rate**: MRL1 = (L1 read misses + L1 write misses) / (L1 reads + L1 writes)
- **Number of writebacks from L1 to the next level**
- **Number of L1 prefetches** (prefetch requests from L1 to the next level, if the prefetch unit is enabled)
- **Number of L2 reads** that did not originate from L1 prefetches (should match b + d: L1 read misses + L1 write misses)
- **Number of L2 read misses** that did not originate from L1 prefetches, excluding such L2 read misses that hit in the stream buffers if the L2 prefetch unit is enabled
- **Number of L2 reads** that originated from L1 prefetches (should match g: L1 prefetches)
- **Number of L2 read misses** that originated from L1 prefetches, excluding such L2 read misses that hit in the stream buffers if the L2 prefetch unit is enabled
- **Number of L2 writes** (should match f: number of writebacks from L1)
- **Number of L2 write misses**, excluding L2 write misses that hit in the stream buffers if the L2 prefetch unit is enabled
- **L2 miss rate** (from the standpoint of stalling the CPU): MRL2 = (item i) / (item h)
- **Number of writebacks from L2 to memory**
- **Number of L2 prefetches** (prefetch requests from L2 to the next level, if the prefetch unit is enabled)
- **Total memory traffic**: number of blocks transferred to/from memory 
  - With L2, should match i + k + m + o + p: all L2 read misses + L2 write misses + writebacks from L2 + L2 prefetches
  - Without L2, should match b + d + f + g: L1 read misses + L1 write misses + writebacks from L1 + L1 prefetches.
