#ifndef KLIKOS_FB_H
#define KLIKOS_FB_H

#include <kernel/types.h>

/* Framebuffer info (QEMU virt GPU / simple framebuffer) */
#define FB_BASE    0x60000000
#define FB_WIDTH   800
#define FB_HEIGHT  600
#define FB_BPP     32
#define FB_STRIDE  (FB_WIDTH * 4)

typedef struct {
    u32 width;
    u32 height;
    u32 bpp;
    u32 stride;
    u8  *base;
} fb_info_t;

extern fb_info_t fb;

void fb_init(void);
void fb_put_pixel(u32 x, u32 y, u32 color);
void fb_fill_rect(u32 x, u32 y, u32 w, u32 h, u32 color);
void fb_clear(u32 color);
void fb_hline(u32 x, u32 y, u32 w, u32 color);
void fb_vline(u32 x, u32 y, u32 h, u32 color);

#endif /* KLIKOS_FB_H */
