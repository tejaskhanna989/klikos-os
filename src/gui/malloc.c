/* malloc.c — Simple heap allocator for GUI widget allocation.
 *
 * Minimal bump allocator. Real OS would have a proper kernel heap.
 */

#include <kernel/types.h>

#define HEAP_SIZE  (1024 * 1024)  /* 1 MiB heap */
#define HEAP_BASE  0x70000000

static u8  heap[HEAP_SIZE];
static u32 heap_offset = 0;

void *malloc(u32 size)
{
    if (heap_offset + size > HEAP_SIZE) return NULL;
    void *ptr = &heap[heap_offset];
    heap_offset += size;
    /* Align to 4 bytes */
    heap_offset = (heap_offset + 3) & ~3;
    return ptr;
}

void free(void *ptr)
{
    /* No-op bump allocator — memory is never freed */
    (void)ptr;
}
