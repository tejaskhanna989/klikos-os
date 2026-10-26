/* fb.c — Framebuffer / console driver.
 *
 * aarch64: uses linear framebuffer (ramfb) at FB_BASE.
 * x86_64:  scans PCI for a display device (ramfb/virtio-gpu) and uses its BAR0.
 *          Falls back to VGA text mode at 0xB8000 if no GPU found.
 */

#include <kernel/fb.h>
#include <kernel/printf.h>
#include <kernel/types.h>

fb_info_t fb;

/* ---------- Minimal PCI config space access via IO ports (x86 only) ---------- */

static inline void io_outb(u16 port, u8 val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline void io_outl(u16 port, u32 val) {
    __asm__ volatile ("outl %0, %1" : : "a"(val), "Nd"(port));
}

static inline u8  io_inb(u16 port) {
    u8 ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline u32 io_inl(u16 port) {
    u32 ret;
    __asm__ volatile ("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static u32 pci_read(u32 bus, u32 dev, u32 func, u32 offset) {
    u32 addr = (1u << 31) | (bus << 16) | (dev << 11) | (func << 8) | (offset & ~3u);
    io_outl(0xCF8, addr);
    return io_inl(0xCFC + (offset & 3));
}

/* ---------- Framebuffer detection ---------- */

#ifdef __aarch64__
void fb_init(void)
{
    fb.width   = FB_WIDTH;
    fb.height  = FB_HEIGHT;
    fb.bpp     = FB_BPP;
    fb.stride  = FB_STRIDE;
    fb.base    = (u8 *)FB_BASE;

    kprintf("[fb] Framebuffer at 0x%x, %ux%u\n",
            (u32)FB_BASE, fb.width, fb.height);

    fb_clear(0x000000);
}
#else
/* x86_64: scan PCI for a display device and use its BAR0 as framebuffer */

static u32 find_fb_address(void)
{
    for (u32 bus = 0; bus < 8; bus++) {
        for (u32 dev = 0; dev < 32; dev++) {
            u32 vendor = pci_read(bus, dev, 0, 0);
            if (vendor == 0xFFFFFFFF) continue;

            for (u32 func = 0; func < 8; func++) {
                u32 v = pci_read(bus, dev, func, 0);
                if (v == 0xFFFFFFFF) continue;

                u32 class = pci_read(bus, dev, func, 0x08) >> 16;
                if (class != 0x03) continue;  /* not a display controller */

                u32 bar0 = pci_read(bus, dev, func, 0x10);
                u32 bar0_addr = bar0 & ~0xF;  /* mask to get base address */
                if (bar0_addr == 0) continue;

                kprintf("[fb] Found display at bus=%u dev=%u func=%u BAR0=0x%x\n",
                        bus, dev, func, bar0_addr);
                return bar0_addr;
            }
        }
    }
    return 0;  /* not found */
}

void fb_init(void)
{
    u32 fb_addr = find_fb_address();

    if (fb_addr) {
        fb.base    = (u8 *)fb_addr;
        kprintf("[fb] PCI framebuffer at 0x%x, %ux%u\n",
                fb_addr, FB_WIDTH, FB_HEIGHT);
    } else {
        /* No GPU found — fall back to VGA text mode */
        fb.base    = (u8 *)0xB8000;
        kprintf("[fb] No GPU found, using VGA text at 0xB8000\n");
    }

    fb.width   = FB_WIDTH;
    fb.height  = FB_HEIGHT;
    fb.bpp     = FB_BPP;
    fb.stride  = FB_STRIDE;

    fb_clear(0x000000);
}
#endif /* __aarch64__ */

void fb_put_pixel(u32 x, u32 y, u32 color)
{
    if (x >= fb.width || y >= fb.height) return;
    u32 *pixel = (u32 *)(fb.base + (y * fb.stride) + (x * 4));
    *pixel = color;
}

void fb_fill_rect(u32 x, u32 y, u32 w, u32 h, u32 color)
{
    for (u32 dy = 0; dy < h; dy++) {
        for (u32 dx = 0; dx < w; dx++) {
            fb_put_pixel(x + dx, y + dy, color);
        }
    }
}

void fb_clear(u32 color)
{
    fb_fill_rect(0, 0, fb.width, fb.height, color);
}

void fb_hline(u32 x, u32 y, u32 w, u32 color)
{
    fb_fill_rect(x, y, w, 1, color);
}

void fb_vline(u32 x, u32 y, u32 h, u32 color)
{
    fb_fill_rect(x, y, 1, h, color);
}
