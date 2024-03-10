#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdalign.h>

#include "memory.h"

// mem_header precedes each allocated memory block
typedef struct mem_header {
    uintptr_t pad;
    struct mem_header *next;
    size_t size;
    bool in_use;
} mem_header;

// Memory is organized in a linked list of memory blocks.
// We keep refs to the head and tail of this list.
mem_header *head = NULL, *tail = NULL;

// mem_alloc allocates memory blocks, or reuses freed blocks.
// The returned memory block is zeroed and it is 8-byte aligned.
// The caller is responsible for aligning user data that needs larger
// alignment.
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

    // Calculate alignment, minimum 8 bytes:
    size_t align = alignof(mem_header) < 8 ? 8 : alignof(mem_header);

    // Allocate a new memory block, include padding bytes to ensure alignment:
    size_t block_size = sizeof(mem_header) + size + align - 1;
    void *res = sbrk(block_size);
    if (res == (void *)-1) {
        exit(1);
    }

    // Calculate padding to align the mem_header
    uintptr_t pad = 0;
    if ((uintptr_t)res % align != 0) {
        pad = align - ((uintptr_t)res % align);
    }
    // Extend size to cover the rest of the padding bytes
    size = block_size - pad - sizeof(mem_header);

    // Initialize the new memory block
    mem_header *hdr = (mem_header *)(res + pad);
    hdr->pad = pad;
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
        if (end == (void *)((uintptr_t)curr->next - curr->next->pad)) {
            mem_header *merged = curr->next;
            curr->size += merged->pad + sizeof(mem_header) + merged->size;
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
        void *res = sbrk(-tail->pad - sizeof(mem_header) - tail->size);
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
        tot += hdr->pad + sizeof(mem_header) + hdr->size;
    }
    return tot;
}

// testmem tests the memory allocator.
void testmem()
{
    size_t mem_start = memusage();

    void *foo = reallocate(NULL, 0, 10);
    fprintf(stderr, "Memusage: %zu\n", memusage());
    void *bar = reallocate(NULL, 0, 20);
    fprintf(stderr, "Memusage: %zu\n", memusage());
    void *moo = reallocate(NULL, 0, 40);
    fprintf(stderr, "Memusage: %zu\n", memusage());
    reallocate(moo, 10, 0);
    fprintf(stderr, "Memusage: %zu\n", memusage());
    reallocate(bar, 10, 0);
    fprintf(stderr, "Memusage: %zu\n", memusage());
    reallocate(foo, 10, 0);
    fprintf(stderr, "Memusage: %zu\n", memusage());

    assert(memusage() == mem_start);

    foo = reallocate(NULL, 0, 10);
    fprintf(stderr, "Memusage: %zu\n", memusage());
    bar = reallocate(NULL, 0, 20);
    fprintf(stderr, "Memusage: %zu\n", memusage());
    moo = reallocate(NULL, 0, 40);
    fprintf(stderr, "Memusage: %zu\n", memusage());
    reallocate(foo, 10, 0);
    fprintf(stderr, "Memusage: %zu\n", memusage());
    reallocate(bar, 10, 0);
    fprintf(stderr, "Memusage: %zu\n", memusage());
    reallocate(moo, 10, 0);
    fprintf(stderr, "Memusage: %zu\n", memusage());

    assert(memusage() == mem_start);
}
