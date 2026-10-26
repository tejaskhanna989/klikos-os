/* mm.c — KlikOS kernel memory manager.
 *
 * Physical page allocator + simple kernel heap (first-fit).
 * Sets up a basic identity-mapped page table for the kernel.
 */

#include <kernel/mm.h>
#include <kernel/printf.h>
#include <kernel/string.h>
#include <kernel/types.h>

/* Page table (4KB aligned, at a fixed high address) */
static u32 *page_table = (u32 *)0x7FFF0000;

/* Physical memory bitmap: 1 bit per 4KB page.
 * Supports up to 1 GiB RAM (262144 pages = 32 KiB bitmap). */
#define MAX_PAGES   262144
#define BITMAP_SIZE (MAX_PAGES / 8)
static u8 page_bitmap[BITMAP_SIZE];
static u32 total_pages = 0;
static u32 free_pages = 0;

/* Kernel heap state */
static u8 heap_buf[KERNEL_HEAP_SIZE] __attribute__((aligned(8)));
static u32 heap_used = 0;

typedef struct {
    u32 size;
    int free;
} heap_hdr_t;

#define HDR_SIZE sizeof(heap_hdr_t)

static void k_memset(u8 *ptr, u8 val, u32 n)
{
    while (n--) *ptr++ = val;
}

/* ---------- Physical page allocator ---------- */

void mm_init(void)
{
    /* Kernel is loaded at 0x40080000. Mark kernel pages as used. */
    u32 kernel_end = 0x40100000 / PAGE_SIZE;   /* conservative */

    k_memset(page_bitmap, 0, BITMAP_SIZE);
    total_pages = MAX_PAGES;
    free_pages = MAX_PAGES;

    for (u32 i = 0; i < kernel_end; i++) {
        page_bitmap[i / 8] |= (1 << (i % 8));
        free_pages--;
    }

    /* Mark page-table page as used */
    page_bitmap[(0x7FFF0000 / PAGE_SIZE) / 8] |= (1 << ((0x7FFF0000 / PAGE_SIZE) % 8));
    free_pages--;

    kprintf("[mm] Initialized: %u MB RAM, %u free pages\n",
            total_pages * PAGE_SIZE / (1024*1024), free_pages);
}

u32 mm_alloc_pages(u32 count)
{
    u32 start = 0;
    u32 found = 0;

    for (u32 i = 0; i < MAX_PAGES; i++) {
        if (!(page_bitmap[i / 8] & (1 << (i % 8)))) {
            if (found == 0) start = i;
            found++;
            if (found == count) {
                for (u32 j = 0; j < count; j++) {
                    u32 pg = start + j;
                    page_bitmap[pg / 8] |= (1 << (pg % 8));
                }
                free_pages -= count;
                return start * PAGE_SIZE;
            }
        } else {
            found = 0;
        }
    }
    return 0; /* out of memory */
}

void mm_free_pages(u32 addr, u32 count)
{
    u32 start = addr / PAGE_SIZE;
    for (u32 j = 0; j < count; j++) {
        u32 pg = start + j;
        page_bitmap[pg / 8] &= ~(1 << (pg % 8));
    }
    free_pages += count;
}

void mm_map_page(u32 vaddr, u32 paddr, u32 flags)
{
    /* Identity map for now (page table not yet enabled in hardware) */
    (void)vaddr; (void)paddr; (void)flags;
}

void mm_unmap_page(u32 vaddr)
{
    (void)vaddr;
}

/* ---------- Kernel heap (first-fit) ---------- */

void *kmalloc(u32 size)
{
    if (size == 0) return NULL;
    size = (size + 7) & ~7;  /* 8-byte align */

    u32 offset = 0;
    while (offset < heap_used) {
        heap_hdr_t *hdr = (heap_hdr_t *)&heap_buf[offset];
        if (hdr->free && hdr->size >= size) {
            hdr->free = 0;
            return (void *)&heap_buf[offset + HDR_SIZE];
        }
        offset += HDR_SIZE + hdr->size;
    }

    /* No fit — extend heap */
    if (heap_used + HDR_SIZE + size > KERNEL_HEAP_SIZE)
        return NULL;

    heap_hdr_t *hdr = (heap_hdr_t *)&heap_buf[heap_used];
    hdr->size = size;
    hdr->free = 0;
    heap_used += HDR_SIZE + size;
    return (void *)&heap_buf[heap_used - size];
}

void kfree(void *ptr)
{
    if (!ptr) return;
    u8 *p = (u8 *)ptr - HDR_SIZE;
    heap_hdr_t *hdr = (heap_hdr_t *)p;
    hdr->free = 1;
}
