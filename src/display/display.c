/* display.c — KlikOS display driver.
 *
 * Framebuffer output via /dev/fb0 or direct PCI BAR0 mmap via /dev/mem.
 */

#define _DEFAULT_SOURCE
#include "display.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <dirent.h>
#include <errno.h>

/* Built-in 5x7 font */
static const uint8_t font5x7[95][5] = {
    {0x00,0x00,0x00,0x00,0x00}, {0x00,0x00,0x5F,0x00,0x00}, {0x00,0x07,0x00,0x07,0x00},
    {0x14,0x7F,0x14,0x7F,0x14}, {0x24,0x2A,0x7F,0x2A,0x12}, {0x23,0x13,0x08,0x64,0x62},
    {0x36,0x49,0x55,0x22,0x50}, {0x00,0x05,0x03,0x00,0x00}, {0x00,0x1C,0x22,0x41,0x00},
    {0x00,0x41,0x22,0x1C,0x00}, {0x08,0x2A,0x1C,0x2A,0x08}, {0x08,0x08,0x3E,0x08,0x08},
    {0x00,0x50,0x30,0x00,0x00}, {0x08,0x08,0x08,0x08,0x08}, {0x00,0x60,0x60,0x00,0x00},
    {0x20,0x10,0x08,0x04,0x02}, {0x3E,0x51,0x49,0x45,0x3E}, {0x00,0x42,0x7F,0x40,0x00},
    {0x42,0x61,0x51,0x49,0x46}, {0x21,0x41,0x45,0x4B,0x31}, {0x18,0x14,0x12,0x7F,0x10},
    {0x27,0x45,0x45,0x45,0x39}, {0x3C,0x4A,0x49,0x49,0x31}, {0x01,0x71,0x09,0x05,0x03},
    {0x36,0x49,0x49,0x49,0x36}, {0x06,0x49,0x49,0x29,0x1E}, {0x00,0x36,0x36,0x00,0x00},
    {0x00,0x56,0x36,0x00,0x00}, {0x00,0x08,0x14,0x22,0x41}, {0x14,0x14,0x14,0x14,0x14},
    {0x00,0x41,0x22,0x14,0x08}, {0x02,0x01,0x51,0x09,0x06}, {0x32,0x49,0x59,0x51,0x3E},
    {0x7E,0x11,0x11,0x11,0x7E}, {0x7F,0x49,0x49,0x49,0x36}, {0x3E,0x41,0x41,0x41,0x22},
    {0x7F,0x41,0x41,0x22,0x1C}, {0x7F,0x49,0x49,0x49,0x41}, {0x7F,0x09,0x09,0x09,0x01},
    {0x3E,0x41,0x41,0x51,0x3E}, {0x7F,0x08,0x08,0x08,0x7F}, {0x00,0x41,0x7F,0x41,0x00},
    {0x20,0x40,0x41,0x3F,0x01}, {0x7F,0x08,0x14,0x22,0x41}, {0x7F,0x40,0x40,0x40,0x40},
    {0x7F,0x02,0x04,0x02,0x7F}, {0x7F,0x04,0x08,0x10,0x7F}, {0x3E,0x41,0x41,0x41,0x3E},
    {0x7F,0x09,0x09,0x09,0x06}, {0x3E,0x41,0x51,0x21,0x5E}, {0x7F,0x09,0x19,0x29,0x46},
    {0x46,0x49,0x49,0x49,0x31}, {0x01,0x01,0x7F,0x01,0x01}, {0x3F,0x40,0x40,0x40,0x3F},
    {0x1F,0x20,0x40,0x20,0x1F}, {0x3F,0x40,0x38,0x40,0x3F}, {0x63,0x14,0x08,0x14,0x63},
    {0x03,0x04,0x78,0x04,0x03}, {0x61,0x51,0x49,0x45,0x43}, {0x00,0x7F,0x41,0x41,0x00},
    {0x02,0x04,0x08,0x10,0x20}, {0x00,0x41,0x41,0x7F,0x00}, {0x04,0x02,0x01,0x02,0x04},
    {0x40,0x40,0x40,0x40,0x40}, {0x00,0x01,0x02,0x04,0x00}, {0x20,0x54,0x54,0x54,0x78},
    {0x7F,0x48,0x44,0x44,0x38}, {0x38,0x44,0x44,0x44,0x20}, {0x38,0x44,0x44,0x48,0x7F},
    {0x38,0x54,0x54,0x54,0x18}, {0x08,0x7E,0x09,0x01,0x00}, {0x0C,0x52,0x52,0x52,0x3E},
    {0x7F,0x08,0x04,0x04,0x78}, {0x00,0x44,0x7D,0x40,0x00}, {0x20,0x40,0x44,0x3D,0x00},
    {0x00,0x7F,0x10,0x28,0x00}, {0x00,0x41,0x7F,0x40,0x00}, {0x7C,0x04,0x18,0x04,0x78},
    {0x7C,0x08,0x04,0x04,0x78}, {0x38,0x44,0x44,0x44,0x38}, {0x7C,0x14,0x14,0x14,0x08},
    {0x08,0x14,0x14,0x18,0x7C}, {0x7C,0x08,0x04,0x04,0x08}, {0x48,0x54,0x54,0x54,0x20},
    {0x04,0x3F,0x44,0x40,0x20}, {0x3C,0x40,0x40,0x20,0x7C}, {0x1C,0x20,0x40,0x20,0x1C},
    {0x3C,0x40,0x30,0x40,0x3C}, {0x44,0x28,0x10,0x28,0x44}, {0x0C,0x50,0x50,0x50,0x3C},
    {0x44,0x64,0x54,0x4C,0x44}, {0x00,0x08,0x36,0x41,0x00}, {0x00,0x00,0x7F,0x00,0x00},
    {0x00,0x41,0x36,0x08,0x00}, {0x08,0x04,0x08,0x10,0x08}
};

static int find_fb_addr(uint32_t *addr, uint32_t *size);

static int find_fb_addr(uint32_t *addr, uint32_t *size);

static void put_pixel_direct(k_display_t *disp, int x, int y, k_color_t color) {
    if (x < 0 || x >= (int)disp->width || y < 0 || y >= (int)disp->height) return;
    uint8_t *pixel = (uint8_t *)disp->fb + (y * disp->stride) + (x * 4);
    pixel[0] = color.b;
    pixel[1] = color.g;
    pixel[2] = color.r;
    pixel[3] = color.a;
}

k_color_t k_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    k_color_t c = {a, r, g, b};
    return c;
}

k_color_t k_color_rgb(uint8_t r, uint8_t g, uint8_t b) {
    k_color_t c = {255, r, g, b};
    return c;
}

k_color_t k_color_blend(k_color_t src, k_color_t dst) {
    if (src.a == 0) return dst;
    if (src.a == 255) return src;
    uint8_t inv_a = 255 - src.a;
    k_color_t out;
    out.r = (src.r * src.a + dst.r * inv_a) / 255;
    out.g = (src.g * src.a + dst.g * inv_a) / 255;
    out.b = (src.b * src.a + dst.b * inv_a) / 255;
    out.a = 255;
    return out;
}

static int find_pci_fb(uint32_t *addr, uint32_t *size) {
    /* Scan PCI sysfs for VGA devices (class 0x03) */
    DIR *d = opendir("/sys/bus/pci/devices");
    if (!d) return -1;

    struct dirent *entry;
    while ((entry = readdir(d)) != NULL) {
        char path[512];
        snprintf(path, sizeof(path), "/sys/bus/pci/devices/%s/class", entry->d_name);
        FILE *f = fopen(path, "r");
        if (!f) continue;

        char cls[16] = {0};
        if (fgets(cls, sizeof(cls), f) == NULL) {
            fclose(f);
            continue;
        }
        fclose(f);

        /* class format: "0x030000" - we want class 0x03 (VGA) */
        unsigned long cls_val;
        if (sscanf(cls, "0x%lx", &cls_val) != 1) continue;
        if ((cls_val >> 16) != 0x03) continue;  // not a VGA device

        /* Read BAR0 */
        snprintf(path, sizeof(path), "/sys/bus/pci/devices/%s/resource0", entry->d_name);
        f = fopen(path, "r");
        if (!f) continue;
        char res[64];
        if (fgets(res, sizeof(res), f) == NULL) {
            fclose(f);
            continue;
        }
        fclose(f);

        unsigned long addr;
        if (sscanf(res, "%lx", if (sscanf(res, "%lx", &addr) != 1) continue;addr) != 1) continue;
        if (addr == 0) continue;

        *addr = addr;
        fclose(f);
        closedir(d);
        printf("[fb] Found PCI framebuffer at 0x%lx\n", addr);
        return 0;
    }

    closedir(d);
    return -1;
}

int k_display_init(k_display_t *disp, const char *device) {
    memset(disp, 0, sizeof(*disp));
    disp->width = 800;
    disp->height = 600;
    disp->stride = disp->width * 4;
    disp->bpp = 32;
    disp->format = 1; /* XRGB8888 */
    disp->fd = -1;
    disp->fb = NULL;

    /* Try /dev/fb0 first (ramfb/simplefb driver) */
    int fbfd = open("/dev/fb0", O_RDWR);
    if (fbfd >= 0) {
        struct fb_var_screeninfo vinfo;
        struct fb_fix_screeninfo finfo;
        if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == 0 &&
            ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == 0) {
            disp->width = vinfo.xres;
            disp->height = vinfo.yres;
            disp->bpp = vinfo.bits_per_pixel;
            disp->stride = finfo.line_length;
            disp->fb = mmap(NULL, finfo.smem_len, PROT_READ | PROT_WRITE,
                           MAP_SHARED, fbfd, 0);
            if (disp->fb != MAP_FAILED) {
                disp->fd = fbfd;
                printf("[fb] Using /dev/fb0 at mmap'd address, %ux%u stride=%u\n",
                        disp->width, disp->height, disp->stride);
                return 0;
            }
            close(fbfd);
        } else {
            close(fbfd);
        }
    }

    /* Fallback: scan PCI for VGA device and mmap BAR0 via /dev/mem */
    uint32_t fb_addr = 0;
    if (find_fb_addr(&fb_addr, NULL) == 0) {
        int memfd = open("/dev/mem", O_RDWR);
        if (memfd >= 0) {
            disp->fb = mmap(NULL, disp->width * disp->height * 4,
                           PROT_READ | PROT_WRITE, MAP_SHARED, memfd, fb_addr);
            if (disp->fb != MAP_FAILED) {
                disp->fd = memfd;
                printf("[fb] Using PCI framebuffer at 0x%x, %ux%u stride=%u\n",
                        fb_addr, disp->width, disp->height, disp->stride);
                return 0;
            }
            close(memfd);
        }
    }

    /* Last resort: allocate memory buffer (not visible on screen) */
    disp->fb = calloc(1, disp->width * disp->height * 4);
    if (!disp->fb) return -1;
    printf("[fb] Warning: no display device, using memory buffer\n");
    return 0;
}

void k_display_cleanup(k_display_t *disp) {
    if (disp->fb && disp->fd >= 0) {
        munmap(disp->fb, disp->height * disp->stride);
        close(disp->fd);
    } else if (disp->fb) {
        free(disp->fb);
    }
    disp->fb = NULL;
}

void k_display_clear(k_display_t *disp, k_color_t color) {
    for (uint32_t y = 0; y < disp->height; y++) {
        for (uint32_t x = 0; x < disp->width; x++) {
            put_pixel_direct(disp, x, y, color);
        }
    }
}

void k_display_flip(k_display_t *disp) {
    (void)disp;
}

void k_draw_rect(k_display_t *disp, k_rect_t rect, k_color_t color) {
    k_draw_fill_rect(disp, (k_rect_t){rect.x, rect.y, rect.w, 1}, color);
    k_draw_fill_rect(disp, (k_rect_t){rect.x, rect.y + rect.h - 1, rect.w, 1}, color);
    k_draw_fill_rect(disp, (k_rect_t){rect.x, rect.y, 1, rect.h}, color);
    k_draw_fill_rect(disp, (k_rect_t){rect.x + rect.w - 1, rect.y, 1, rect.h}, color);
}

void k_draw_fill_rect(k_display_t *disp, k_rect_t rect, k_color_t color) {
    for (int32_t y = rect.y; y < rect.y + rect.h; y++) {
        for (int32_t x = rect.x; x < rect.x + rect.w; x++) {
            put_pixel_direct(disp, x, y, color);
        }
    }
}

void k_draw_round_rect(k_display_t *disp, k_rect_t rect, int radius, k_color_t color) {
    k_draw_fill_rect(disp, (k_rect_t){rect.x + radius, rect.y, rect.w - 2 * radius, rect.h}, color);
    k_draw_fill_rect(disp, (k_rect_t){rect.x, rect.y + radius, radius, rect.h - 2 * radius}, color);
    k_draw_fill_rect(disp, (k_rect_t){rect.x + rect.w - radius, rect.y + radius, radius, rect.h - 2 * radius}, color);
    k_draw_fill_circle(disp, rect.x + radius, rect.y + radius, radius, color);
    k_draw_fill_circle(disp, rect.x + rect.w - radius, rect.y + radius, radius, color);
    k_draw_fill_circle(disp, rect.x + radius, rect.y + rect.h - radius, radius, color);
    k_draw_fill_circle(disp, rect.x + rect.w - radius, rect.y + rect.h - radius, radius, color);
}

void k_draw_fill_round_rect(k_display_t *disp, k_rect_t rect, int radius, k_color_t color) {
    k_draw_round_rect(disp, rect, radius, color);
}

void k_draw_circle(k_display_t *disp, int cx, int cy, int r, k_color_t color) {
    int x = 0, y = r;
    int d = 3 - 2 * r;
    while (x <= y) {
        put_pixel_direct(disp, cx + x, cy + y, color);
        put_pixel_direct(disp, cx - x, cy + y, color);
        put_pixel_direct(disp, cx + x, cy - y, color);
        put_pixel_direct(disp, cx - x, cy - y, color);
        put_pixel_direct(disp, cx + y, cy + x, color);
        put_pixel_direct(disp, cx - y, cy + x, color);
        put_pixel_direct(disp, cx + y, cy - x, color);
        put_pixel_direct(disp, cx - y, cy - x, color);
        if (d < 0) d += 4 * x + 6;
        else { d += 4 * (x - y) + 10; y--; }
        x++;
    }
}

void k_draw_fill_circle(k_display_t *disp, int cx, int cy, int r, k_color_t color) {
    for (int dy = -r; dy <= r; dy++) {
        for (int dx = -r; dx <= r; dx++) {
            if (dx*dx + dy*dy <= r*r) {
                put_pixel_direct(disp, cx + dx, cy + dy, color);
            }
        }
    }
}

void k_draw_line(k_display_t *disp, int x1, int y1, int x2, int y2, k_color_t color) {
    int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
    int dy = abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2;
    while (1) {
        put_pixel_direct(disp, x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        int e2 = err;
        if (e2 > -dx) { err -= dy; x1 += sx; }
        if (e2 < dy) { err += dx; y1 += sy; }
    }
}

void k_draw_pixel(k_display_t *disp, int x, int y, k_color_t color) {
    put_pixel_direct(disp, x, y, color);
}

void k_draw_char(k_display_t *disp, int x, int y, char c, k_color_t color, int scale) {
    if (c < 32 || c > 126) c = '?';
    const uint8_t *glyph = font5x7[c - 32];
    for (int col = 0; col < 5; col++) {
        uint8_t bits = glyph[col];
        for (int row = 0; row < 7; row++) {
            if (bits & (1 << row)) {
                k_draw_fill_rect(disp,
                    (k_rect_t){x + col * scale, y + row * scale, scale, scale}, color);
            }
        }
    }
}

void k_draw_text(k_display_t *disp, int x, int y, const char *text, k_color_t color, int scale) {
    int cx = x;
    while (*text) {
        if (*text == ' ') {
            cx += 6 * scale;
        } else {
            k_draw_char(disp, cx, y, *text, color, scale);
            cx += 6 * scale;
        }
        text++;
    }
}

int k_text_width(const char *text, int scale) {
    int w = 0;
    while (*text) {
        if (*text == ' ') w += 6 * scale;
        else w += 6 * scale;
        text++;
    }
    return w;
}

void k_gradient_rect(k_display_t *disp, int x, int y, int w, int h, k_color_t c1, k_color_t c2, int vertical) {
    uint32_t r1 = c1.r, g1 = c1.g, b1 = c1.b;
    uint32_t r2 = c2.r, g2 = c2.g, b2 = c2.b;
    for (uint32_t i = 0; i < (uint32_t)h; i++) {
        uint32_t t = vertical ? (i * 255 / h) : 0;
        uint32_t r = r1 + (r2 - r1) * t / 255;
        uint32_t g = g1 + (g2 - g1) * t / 255;
        uint32_t b = b1 + (b2 - b1) * t / 255;
        k_color_t color = k_color((uint8_t)r, (uint8_t)g, (uint8_t)b, 255);
        k_draw_fill_rect(disp, (k_rect_t){x, y + i, w, 1}, color);
    }
}
