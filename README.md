# Custom Heap Allocator with Mark-and-Sweep Garbage Collector in C

This is a lightweight custom memory allocator with mark-and-sweep garbage collection, written in pure C. It simulates how malloc/free and GC work under the hood — ideal for systems programmers, compiler writers, and students exploring memory management internals.

This project implements a basic heap memory allocator in C. It features manual allocation (`heap_alloc`), deallocation (`heap_free`), and an automatic mark-and-sweep garbage collector (`heap_collect`). The system manages a fixed-size memory region and serves as a practical example of fundamental memory management and garbage collection concepts.

## Features

* **Fixed-Size Heap:** Manages a predefined memory pool (`HEAP_CAP_BYTES = 640000` bytes).
* **Word-Aligned Allocations:** All allocations are aligned to `sizeof(uintptr_t)`.
* **`heap_alloc(size_t size_bytes)`:**
    * Allocates a block of memory of at least `size_bytes`.
    * Converts requested bytes into words (units of `sizeof(uintptr_t)`).
    * Merges adjacent free blocks in `freed_chunks` to combat fragmentation before searching.
    * Uses a first-fit strategy to find a suitable free block.
    * If a block is larger than needed, it's split; the allocated part goes to `alloced_chunks`, and the remainder stays in `freed_chunks`.
* **`heap_free(void* ptr)`:**
    * Frees a previously allocated memory block pointed to by `ptr`.
    * Moves the corresponding chunk from `alloced_chunks` to `freed_chunks`.
* **`heap_collect()`:**
    * Implements a mark-and-sweep garbage collector.
    * **Mark Phase:** Identifies all reachable memory blocks by scanning the program's stack and recursively following pointers found within other reachable heap objects.
    * **Sweep Phase:** Reclaims all allocated memory blocks that were not marked as reachable.
* **Chunk Management:**
    * Uses `chunk_list` structures (`alloced_chunks`, `freed_chunks`) to track memory blocks.
    * `alloced_chunks`: Stores currently allocated memory blocks, sorted by their start address.
    * `freed_chunks`: Stores available (free) memory blocks, sorted by their start address.
    * `chunk_list_find()`: Efficiently finds a chunk by its start pointer using binary search.
    * `chunk_list_insert()`: Inserts a new chunk into a list while maintaining sorted order.
    * `chunk_list_remove()`: Removes a chunk from a list.
    * `chunk_list_merge()`: Coalesces adjacent free chunks in a list to form larger free blocks, reducing fragmentation.
* **Debugging Macro:**
    * The `todo` macro is used as a debugging aid to print a message and abort if a certain part of the code (assumed to be unfinished or an error state) is reached.

## File Structure

* **`heap.h`**: The header file.
    * Defines constants for heap capacity (`HEAP_CAP_BYTES`, `HEAP_CAP`) and chunk list capacities (`CHUNK_LIST_CAP`).
    * Declares the global heap array (`heap`), `stack_base` pointer.
    * Defines data structures:
        * `chunk`: Represents a block of memory with a `start` pointer and `size` (in words).
        * `chunk_list`: Manages a list of `chunk`s.
    * Declares function prototypes for heap operations and chunk list management.
* **`heap.c`**: The implementation file.
    * Contains the definitions of the global heap array and chunk lists (`alloced_chunks`, `freed_chunks`, `temp_chunks`).
    * Implements the logic for `heap_alloc`, `heap_free`, `heap_collect`, and all `chunk_list_*` helper functions.
* **`main.c`**: An example program.
    * Demonstrates the usage of the custom heap allocator and garbage collector.
    * It involves allocating memory for a binary tree, performing garbage collection with reachable objects, making objects unreachable, and then performing garbage collection again to reclaim memory.

## Key Concepts and How It Works

### Memory Representation
* A global array `uintptr_t heap[HEAP_CAP]` acts as the raw memory pool. `HEAP_CAP` is the total number of `uintptr_t` words the heap can hold.
* Memory within this pool is managed in **chunks**. A `chunk` is defined by its `start` pointer (an address within the `heap` array) and its `size` (number of `uintptr_t` words it occupies).

### Chunk Sizes
* **Important Note:** Throughout the system, `chunk.size` is consistently treated as the number of `uintptr_t` elements (words), **not bytes**.
* The initial setup of `freed_chunks` in the provided `heap.c` is:
    ```c
    chunk_list freed_chunks = {
        .count = 1,
        .chunks = {[0] = {.start = heap, .size = sizeof(heap)}}, // sizeof(heap) is in bytes
    };
    ```
    For consistency with the rest of the system where chunk sizes are in words, this should ideally be:
    ```c
    chunk_list freed_chunks = {
        .count = 1,
        .chunks = {[0] = {.start = heap, .size = HEAP_CAP}}, // HEAP_CAP is in words
    };
    ```
    The README will describe the system assuming `chunk.size` is in words, as this aligns with pointer arithmetic and size calculations in functions like `heap_alloc` and `mark_region`.

### Allocation (`heap_alloc`)
1.  The requested `size_bytes` is converted to `size_words`.
2.  `freed_chunks` is first processed by `chunk_list_merge` (using `temp_chunks` as auxiliary storage) to combine any adjacent free blocks. This helps in finding larger contiguous blocks and reduces external fragmentation.
3.  The (merged) `freed_chunks` list is iterated to find the **first chunk** large enough to satisfy `size_words` (First-Fit strategy).
4.  If a suitable free chunk is found:
    * It's removed from `freed_chunks`.
    * The required `size_words` are allocated from this chunk. This new allocated block is added to `alloced_chunks` (which is kept sorted by address).
    * If there's any remaining space in the free chunk after allocation (`rest_size > 0`), this smaller remaining chunk is re-inserted into `freed_chunks`.
    * A pointer to the start of the allocated block is returned.
5.  If no suitable block is found, `NULL` is returned.

### Deallocation (`heap_free`)
1.  If `ptr` is not `NULL`:
2.  It searches for the chunk corresponding to `ptr` in `alloced_chunks` using `chunk_list_find` (binary search).
3.  If found, the chunk is removed from `alloced_chunks` and inserted into `freed_chunks` (maintaining sorted order).

### Garbage Collection (`heap_collect`)
This system uses a Mark-and-Sweep garbage collector.
1.  **Initialization:**
    * The `stack_base` global variable must be initialized at the beginning of the `main` function (or the root function of the user program) to point to the base of the stack. This is done using `stack_base = (const uintptr_t*)__builtin_frame_address(0);`.
    * An array `reachable_chunks` (boolean flags, one per potential allocated chunk) is cleared.
2.  **Mark Phase:**
    * The `mark_region` function is called, starting with the current stack region: from the current frame address (`__builtin_frame_address(0)`) up to `stack_base`.
    * `mark_region(start_addr, end_addr)` iterates through all `uintptr_t` values in the memory range `[start_addr, end_addr)`.
    * Each value `*start` is treated as a potential pointer `p = (const uintptr_t*)*start`.
    * The system checks if `p` points into any chunk currently in `alloced_chunks`.
    * If `p` points into an allocated chunk and that chunk hasn't been marked `reachable` yet:
        * The chunk is marked as `reachable_chunks[index] = true`.
        * `mark_region` is called **recursively** for the memory region of this newly marked reachable chunk (`Chunk.start` to `Chunk.start + Chunk.size`). This ensures that objects referenced by other heap objects are also marked.
3.  **Sweep Phase:**
    * After the mark phase completes, the `alloced_chunks` list is iterated.
    * Any chunk `alloced_chunks.chunks[i]` for which `reachable_chunks[i]` is `false` is considered garbage.
    * The start pointers of these unreachable chunks are collected into a temporary `to_free` array.
    * Finally, `heap_free()` is called for each pointer in the `to_free` array, moving these chunks to the `freed_chunks` list.

## How to Build and Run

This project can be compiled using a C compiler like GCC.

```bash
# Compile the heap implementation and the example main program
gcc -o heap_demo main.c heap.c

# Run the demonstration
./heap_demo
```
###Example Output of the main.c file:
```bash

        g
    d
        f
a
        f
    c
        e

-----------------------------
Alloced Chunk (7):
Start: 00408088, size:3
Start: 00408094, size:3
Start: 004080A0, size:3
Start: 004080AC, size:3
Start: 004080B8, size:3
Start: 004080C4, size:3
Start: 004080D0, size:3
freed Chunk (11):
Start: 00408040, size:1
Start: 00408044, size:1
Start: 00408048, size:1
Start: 0040804C, size:1
Start: 00408050, size:2
Start: 00408058, size:2
Start: 00408060, size:2
Start: 00408068, size:2
Start: 00408070, size:3
Start: 0040807C, size:3
Start: 004080DC, size:159961

-----------------------------
Alloced Chunk (1):
Start: 00408040, size:1
freed Chunk (1):
Start: 00408044, size:159999
```
