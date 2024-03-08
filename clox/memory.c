#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "memory.h"

// mem_header precedes each allocated memory block
typedef struct mem_header {
    struct mem_header *next;
    size_t size;
    bool in_use;
} mem_header;

// Memory is organized in a linked list of memory blocks.
// We keep refs to the head and tail of this list.
mem_header *head = NULL, *tail = NULL;

// mem_alloc allocates memory blocks, or reuses freed blocks.
// The returned memory block is zeroed.
static void *mem_alloc(size_t size)
{
    // Look for a free memory block
    for (mem_header *hdr = head; hdr; hdr = hdr->next) {
        if (!hdr->in_use && hdr->size <= size) {
            hdr->in_use = true;
            memset(hdr + 1, 0, hdr->size);
            return (void *)(hdr + 1);
        }
    }

    // Allocate a new memory block:
    void *res = sbrk(sizeof(mem_header) + size);
    if (res == (void *)-1) {
        exit(1);
    }

    // Initialize the new memory block
    mem_header *hdr = (mem_header *)res;
    hdr->next = NULL;
    hdr->size = size;
    hdr->in_use = true;
    memset(hdr + 1, 0, hdr->size);

    // Update head and tail of linked list
    if (!head) {
        head = hdr;
    }
    if (tail) {
        tail->next = hdr;
    }
    tail = hdr;

    return (void *)(hdr + 1);
}

// mem_free frees memory from the custom memory allocator.
// It merges adjacent free memory blocks and returns memory to the OS if
// feasible.
static void mem_free(void *block)
{
    if (!block) {
        return;
    }

    // Mark block as free
    mem_header *hdr = (mem_header *)block - 1;
    hdr->in_use = false;

    // Defragment free memory
    mem_header *curr = head, *prev = NULL;
    while (curr) {
        if (!curr->next) break;

        if (curr->in_use || curr->next->in_use) {
             prev = curr;
             curr = curr->next;
             continue;
        }

        // This and the next block are free.
        // Merge them if they are contiguous.
        void *end = (char *)(curr + 1) + curr->size;
        if (end == (void *)curr->next) {
            mem_header *merged = curr->next;
            curr->size += sizeof(mem_header) + merged->size;
            curr->next = merged->next;
            if (tail == merged) {
                tail = curr;
            }
        }
    }

    if (tail->in_use) {
        return;
    }

    // Tail block is not in use. Release it back to the OS.
    void *prog_brk = sbrk(0);
    if (prog_brk == (void*)-1) {
        // ???
        exit(1);
    }

    if (((char*)(tail + 1) + tail->size) == prog_brk) {
        void *res = sbrk(-sizeof(mem_header) - tail->size);
        if (res == (void*)-1) {
            // ???
            exit(1);
        }
        tail = prev;
        if (tail) {
            tail->next = NULL;
        } else {
            head = NULL;
        }
    }
}

// reallocate is the only external API to the memory allocator.
void* reallocate(void* pointer, size_t oldSize, size_t newSize)
{
    if (newSize == 0) {
        mem_free(pointer);
        return NULL;
    }

    if (pointer == NULL) {
        return mem_alloc(newSize);
    }

    mem_header *hdr = (mem_header*)pointer - 1;

    if (newSize <= hdr->size) {
        // Reuse current memory block.
        // We could split the block
        return pointer;
    }

    // newSize doesn't fit in the current block.
    // Allocate a new block, move the data over and free the old block.
    void *newPtr = mem_alloc(newSize);
    memcpy(newPtr, pointer, oldSize);
    mem_free(pointer);
    return newPtr;
}

// memusage walks the memory block list and reports how much memory has been
// allocated.
static size_t memusage()
{
    if (!head)
        return 0;

    size_t tot = 0;
    for (mem_header *hdr = head; hdr; hdr = hdr->next) {
        tot += sizeof(mem_header) + hdr->size;
    }
    return tot;
}

// testmem tests the memory allocator.
void testmem()
{
    size_t mem_start = memusage();

    void *foo = reallocate(NULL, 0, 10);
    void *bar = reallocate(NULL, 0, 20);
    void *moo = reallocate(NULL, 0, 40);
    reallocate(moo, 10, 0);
    reallocate(bar, 10, 0);
    reallocate(foo, 10, 0);

    assert(memusage() == mem_start);

    foo = reallocate(NULL, 0, 10);
    bar = reallocate(NULL, 0, 20);
    moo = reallocate(NULL, 0, 40);
    reallocate(foo, 10, 0);
    reallocate(bar, 10, 0);
    reallocate(moo, 10, 0);

    assert(memusage() == mem_start);
}
