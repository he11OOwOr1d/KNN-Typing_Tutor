#include "memory.h"

#define MEMORY_SIZE 10000

static char MEMORY[MEMORY_SIZE];
static int offset = 0;

void* my_alloc(int size) {
    if (offset + size >= MEMORY_SIZE) return 0;

    void* ptr = &MEMORY[offset];
    offset += size;
    return ptr;
}

void my_free(void* ptr) {
    // simple allocator → no free
}
