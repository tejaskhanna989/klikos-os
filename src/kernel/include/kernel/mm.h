#ifndef KLIKOS_KERNEL_MM_H
#define KLIKOS_KERNEL_MM_H

#include <kernel/types.h>

#define PAGE_SIZE      4096
#define PAGE_SHIFT     12
#define KERNEL_HEAP    0x80000000
#define KERNEL_HEAP_SIZE (64 * 1024 * 1024)  /* 64 MiB kernel heap */

void mm_init(void);
void *kmalloc(u32 size);
void kfree(void *ptr);
u32  mm_alloc_pages(u32 count);
void mm_free_pages(u32 addr, u32 count);
void mm_map_page(u32 vaddr, u32 paddr, u32 flags);
void mm_unmap_page(u32 vaddr);

#endif /* KLIKOS_KERNEL_MM_H */
