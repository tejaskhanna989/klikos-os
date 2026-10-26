/* draw.c — KlikOS GUI drawing primitives.
 *
 * aarch64: pixel-based framebuffer drawing.
 * x86_64:  VGA text mode character-based drawing.
 */

#include <kernel/fb.h>
#include <kernel/string.h>
#include <gui/draw.h>

#ifdef __aarch64__
/* ---- ARM64: pixel-based framebuffer drawing ---- */

/* 5x7 bitmap font (rows of pixels, MSB first) */
static const u8 font5x7[95][5] = {
    {0x00,0x00,0x00,0x00,0x00}, /* space */
    {0x00,0x00,0x5F,0x00,0x00}, /* ! */
    {0x00,0x07,0x00,0x07,0x00}, /* " */
    {0x14,0x7F,0x14,0x7F,0x14}, /* # */
    {0x24,0x2A,0x7F,0x2A,0x12}, /* $ */
    {0x23,0x13,0x08,0x64,0x62}, /* % */
    {0x36,0x49,0x55,0x22,0x50}, /* & */
    {0x00,0x05,0x03,0x00,0x00}, /* ' */
    {0x00,0x1C,0x22,0x41,0x00}, /* ( */
    {0x00,0x41,0x22,0x1C,0x00}, /* ) */
    {0x08,0x2A,0x1C,0x2A,0x08}, /* * */
    {0x08,0x08,0x3E,0x08,0x08}, /* + */
    {0x00,0x50,0x30,0x00,0x00}, /* , */
    {0x08,0x08,0x08,0x08,0x08}, /* - */
    {0x00,0x60,0x60,0x00,0x00}, /* . */
    {0x20,0x10,0x08,0x04,0x02}, /* / */
    {0x3E,0x51,0x49,0x45,0x3E}, /* 0 */
    {0x00,0x42,0x7F,0x40,0x00}, /* 1 */
    {0x42,0x61,0x51,0x49,0x46}, /* 2 */
    {0x21,0x41,0x45,0x4B,0x31}, /* 3 */
    {0x18,0x14,0x12,0x7F,0x10}, /* 4 */
    {0x27,0x45,0x45,0x45,0x39}, /* 5 */
    {0x3C,0x4A,0x49,0x49,0x31}, /* 6 */
    {0x01,0x71,0x09,0x05,0x03}, /* 7 */
    {0x36,0x49,0x49,0x49,0x36}, /* 8 */
    {0x06,0x49,0x49,0x29,0x1E}, /* 9 */
    {0x00,0x36,0x36,0x00,0x00}, /* : */
    {0x00,0x56,0x36,0x00,0x00}, /* ; */
    {0x00,0x08,0x14,0x22,0x41}, /* < */
    {0x14,0x14,0x14,0x14,0x14}, /* = */
    {0x00,0x41,0x22,0x14,0x08}, /* > */
    {0x02,0x01,0x51,0x09,0x06}, /* ? */
    {0x32,0x49,0x59,0x51,0x3E}, /* @ */
    {0x7E,0x11,0x11,0x11,0x7E}, /* A */
    {0x7F,0x49,0x49,0x49,0x36}, /* B */
    {0x3E,0x41,0x41,0x41,0x22}, /* C */
    {0x7F,0x41,0x41,0x22,0x1C}, /* D */
    {0x7F,0x49,0x49,0x49,0x41}, /* E */
    {0x7F,0x09,0x09,0x09,0x01}, /* F */
    {0x3E,0x41,0x41,0x51,0x3E}, /* G */
    {0x7F,0x08,0x08,0x08,0x7F}, /* H */
    {0x00,0x41,0x7F,0x41,0x00}, /* I */
    {0x20,0x40,0x41,0x3F,0x01}, /* J */
    {0x7F,0x08,0x14,0x22,0x41}, /* K */
    {0x7F,0x40,0x40,0x40,0x40}, /* L */
    {0x7F,0x02,0x04,0x02,0x7F}, /* M */
    {0x7F,0x04,0x08,0x10,0x7F}, /* N */
    {0x3E,0x41,0x41,0x41,0x3E}, /* O */
    {0x7F,0x09,0x09,0x09,0x06}, /* P */
    {0x3E,0x41,0x51,0x21,0x5E}, /* Q */
    {0x7F,0x09,0x19,0x29,0x46}, /* R */
    {0x46,0x49,0x49,0x49,0x31}, /* S */
    {0x01,0x01,0x7F,0x01,0x01}, /* T */
    {0x3F,0x40,0x40,0x40,0x3F}, /* U */
    {0x1F,0x20,0x40,0x20,0x1F}, /* V */
    {0x3F,0x40,0x38,0x40,0x3F}, /* W */
    {0x63,0x14,0x08,0x14,0x63}, /* X */
    {0x03,0x04,0x78,0x04,0x03}, /* Y */
    {0x61,0x51,0x49,0x45,0x43}, /* Z */
    {0x00,0x7F,0x41,0x41,0x00}, /* [ */
    {0x02,0x04,0x08,0x10,0x20}, /* \ */
    {0x00,0x41,0x41,0x7F,0x00}, /* ] */
    {0x04,0x02,0x01,0x02,0x04}, /* ^ */
    {0x40,0x40,0x40,0x40,0x40}, /* _ */
    {0x00,0x01,0x02,0x04,0x00}, /* ` */
    {0x20,0x54,0x54,0x54,0x78}, /* a */
    {0x7F,0x48,0x44,0x44,0x38}, /* b */
    {0x38,0x44,0x44,0x44,0x20}, /* c */
    {0x38,0x44,0x44,0x48,0x7F}, /* d */
    {0x38,0x54,0x54,0x54,0x18}, /* e */
    {0x08,0x7E,0x09,0x01,0x00}, /* f */
    {0x0C,0x52,0x52,0x52,0x3E}, /* g */
    {0x7F,0x08,0x04,0x04,0x78}, /* h */
    {0x00,0x44,0x7D,0x40,0x00}, /* i */
    {0x20,0x40,0x44,0x3D,0x00}, /* j */
    {0x00,0x7F,0x10,0x28,0x00}, /* k */
    {0x00,0x41,0x7F,0x40,0x00}, /* l */
    {0x7C,0x04,0x18,0x04,0x78}, /* m */
    {0x7C,0x08,0x04,0x04,0x78}, /* n */
    {0x38,0x44,0x44,0x44,0x38}, /* o */
    {0x7C,0x14,0x14,0x14,0x08}, /* p */
    {0x08,0x14,0x14,0x18,0x7C}, /* q */
    {0x7C,0x08,0x04,0x04,0x08}, /* r */
    {0x48,0x54,0x54,0x54,0x20}, /* s */
    {0x04,0x3F,0x44,0x40,0x20}, /* t */
    {0x3C,0x40,0x40,0x20,0x7C}, /* u */
    {0x1C,0x20,0x40,0x20,0x1C}, /* v */
    {0x3C,0x40,0x30,0x40,0x3C}, /* w */
    {0x44,0x28,0x10,0x28,0x44}, /* x */
    {0x0C,0x50,0x50,0x50,0x3C}, /* y */
    {0x44,0x64,0x54,0x4C,0x44}, /* z */
    {0x00,0x08,0x36,0x41,0x00}, /* { */
    {0x00,0x00,0x7F,0x00,0x00}, /* | */
    {0x00,0x41,0x36,0x08,0x00}, /* } */
    {0x08,0x04,0x08,0x10,0x08}  /* ~ */
};

static u32 screen_w = FB_WIDTH;
static u32 screen_h = FB_HEIGHT;

/* Forward declaration */
static u32 k_sqrt(u32 n);

void k_clear(u32 color) {
    fb_fill_rect(0, 0, screen_w, screen_h, color);
}

void k_fill_rect(u32 x, u32 y, u32 w, u32 h, u32 color) {
    fb_fill_rect(x, y, w, h, color);
}

void k_draw_rect(u32 x, u32 y, u32 w, u32 h, u32 color) {
    fb_fill_rect(x, y, w, 1, color);
    fb_fill_rect(x, y + h - 1, w, 1, color);
    fb_fill_rect(x, y, 1, h, color);
    fb_fill_rect(x + w - 1, y, 1, h, color);
}

void k_draw_line(u32 x1, u32 y1, u32 x2, u32 y2, u32 color) {
    int dx = (int)x2 - (int)x1;
    int dy = (int)y2 - (int)y1;
    int sx = (dx < 0) ? -1 : 1;
    int sy = (dy < 0) ? -1 : 1;
    dx = (dx < 0) ? -dx : dx;
    dy = (dy < 0) ? -dy : dy;
    int err = dx - dy;
    u32 cx = x1, cy = y1;
    while (1) {
        fb_put_pixel(cx, cy, color);
        if (cx == x2 && cy == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; cx += sx; }
        if (e2 < dx) { err += dx; cy += sy; }
    }
}

void k_fill_round_rect(u32 x, u32 y, u32 w, u32 h, u32 radius, u32 color) {
    fb_fill_rect(x + radius, y, w - 2 * radius, h, color);
    fb_fill_rect(x, y + radius, radius, h - 2 * radius, color);
    fb_fill_rect(x + w - radius, y + radius, radius, h - 2 * radius, color);

    for (u32 dy = 0; dy < radius; dy++) {
        u32 dx = k_sqrt(radius * radius - dy * dy);
        fb_fill_rect(x + radius - dx, y + dy, dx + 1, 1, color);
        fb_fill_rect(x + radius - dx, y + h - 1 - dy, dx + 1, 1, color);
        fb_fill_rect(x + w - radius + dx, y + dy, dx + 1, 1, color);
        fb_fill_rect(x + w - radius + dx, y + h - 1 - dy, dx + 1, 1, color);
    }
}

void k_draw_circle(u32 cx, u32 cy, u32 r, u32 color) {
    int x = 0, y = (int)r;
    int d = 3 - 2 * r;
    while (x <= y) {
        fb_put_pixel(cx + x, cy + y, color);
        fb_put_pixel(cx - x, cy + y, color);
        fb_put_pixel(cx + x, cy - y, color);
        fb_put_pixel(cx - x, cy - y, color);
        fb_put_pixel(cx + y, cy + x, color);
        fb_put_pixel(cx - y, cy + x, color);
        fb_put_pixel(cx + y, cy - x, color);
        fb_put_pixel(cx - y, cy - x, color);
        if (d < 0) d += 4 * x + 6;
        else { d += 4 * (x - y) + 10; y--; }
        x++;
    }
}

void k_fill_circle(u32 cx, u32 cy, u32 r, u32 color) {
    for (u32 dy = 0; dy <= r; dy++) {
        u32 dx = k_sqrt(r * r - dy * dy);
        fb_fill_rect(cx - dx, cy - dy, dx * 2 + 1, 1, color);
        fb_fill_rect(cx - dx, cy + dy, dx * 2 + 1, 1, color);
    }
}

void k_draw_char(u32 x, u32 y, char c, u32 color, u32 scale) {
    if (c < 32 || c > 126) c = '?';
    u8 glyph[5];
    for (int i = 0; i < 5; i++) glyph[i] = font5x7[c - 32][i];

    for (int col = 0; col < 5; col++) {
        u8 bits = glyph[col];
        for (int row = 0; row < 7; row++) {
            if (bits & (1 << row)) {
                for (u32 sx = 0; sx < scale; sx++) {
                    for (u32 sy = 0; sy < scale; sy++) {
                        fb_put_pixel(x + col * scale + sx, y + row * scale + sy, color);
                    }
                }
            }
        }
    }
}

void k_draw_text(u32 x, u32 y, const char *s, u32 color, u32 scale) {
    u32 cx = x;
    while (*s) {
        if (*s == ' ') {
            cx += 6 * scale;
        } else {
            k_draw_char(cx, y, *s, color, scale);
            cx += 6 * scale;
        }
        s++;
    }
}

u32 k_text_width(const char *s, u32 scale) {
    u32 w = 0;
    while (*s) {
        if (*s == ' ') w += 6 * scale;
        else w += 6 * scale;
        s++;
    }
    return w;
}

void k_gradient_rect(u32 x, u32 y, u32 w, u32 h, u32 c1, u32 c2, int vertical) {
    u32 r1 = (c1 >> 16) & 0xFF, g1 = (c1 >> 8) & 0xFF, b1 = c1 & 0xFF;
    u32 r2 = (c2 >> 16) & 0xFF, g2 = (c2 >> 8) & 0xFF, b2 = c2 & 0xFF;
    for (u32 i = 0; i < h; i++) {
        u32 t = vertical ? (i * 255 / h) : 0;
        u32 r = r1 + (r2 - r1) * t / 255;
        u32 g = g1 + (g2 - g1) * t / 255;
        u32 b = b1 + (b2 - b1) * t / 255;
        u32 color = (r << 16) | (g << 8) | b;
        fb_fill_rect(x, y + i, w, 1, color);
    }
}

/* Simple integer square root */
static u32 k_sqrt(u32 n) {
    if (n == 0) return 0;
    u32 x = n;
    u32 y = (x + 1) / 2;
    while (y < x) {
        x = y;
        y = (x + n / x) / 2;
    }
    return x;
}

#else
/* ---- x86_64: VGA text mode drawing ---- */

#include <kernel/vga.h>

static u32 screen_w = VGA_WIDTH;
static u32 screen_h = VGA_HEIGHT;

void k_clear(u32 color) {
    enum vga_color fg = (enum vga_color)(color & 0xF);
    enum vga_color bg = (enum vga_color)((color >> 4) & 0xF);
    u8 attr = vga_entry_color(fg, bg);
    for (u32 y = 0; y < VGA_HEIGHT; y++)
        for (u32 x = 0; x < VGA_WIDTH; x++)
            vga_put_at(x, y, ' ', attr);
}

void k_fill_rect(u32 x, u32 y, u32 w, u32 h, u32 color) {
    enum vga_color fg = (enum vga_color)(color & 0xF);
    enum vga_color bg = (enum vga_color)((color >> 4) & 0xF);
    u8 attr = vga_entry_color(fg, bg);
    for (u32 dy = 0; dy < h; dy++)
        for (u32 dx = 0; dx < w; dx++)
            if (x + dx < VGA_WIDTH && y + dy < VGA_HEIGHT)
                vga_put_at(x + dx, y + dy, (char)0xDB, attr);
}

void k_draw_rect(u32 x, u32 y, u32 w, u32 h, u32 color) {
    enum vga_color fg = (enum vga_color)(color & 0xF);
    u8 attr = vga_entry_color(fg, VGA_COLOR_BLACK);
    for (u32 i = x; i < x + w && i < VGA_WIDTH; i++) {
        if (y < VGA_HEIGHT) vga_put_at(i, y, (char)0xCD, attr);
        if (y + h - 1 < VGA_HEIGHT) vga_put_at(i, y + h - 1, (char)0xCD, attr);
    }
    for (u32 i = y; i < y + h && i < VGA_HEIGHT; i++) {
        if (x < VGA_WIDTH) vga_put_at(x, i, (char)0xBA, attr);
        if (x + w - 1 < VGA_WIDTH) vga_put_at(x + w - 1, i, (char)0xBA, attr);
    }
    /* Corners */
    if (x < VGA_WIDTH && y < VGA_HEIGHT) vga_put_at(x, y, (char)0xC9, attr);
    if (x + w - 1 < VGA_WIDTH && y < VGA_HEIGHT) vga_put_at(x + w - 1, y, (char)0xBB, attr);
    if (x < VGA_WIDTH && y + h - 1 < VGA_HEIGHT) vga_put_at(x, y + h - 1, (char)0xC8, attr);
    if (x + w - 1 < VGA_WIDTH && y + h - 1 < VGA_HEIGHT) vga_put_at(x + w - 1, y + h - 1, (char)0xBC, attr);
}

void k_draw_line(u32 x1, u32 y1, u32 x2, u32 y2, u32 color) {
    (void)x1; (void)y1; (void)x2; (void)y2; (void)color;
    /* Not needed for text mode */
}

void k_fill_round_rect(u32 x, u32 y, u32 w, u32 h, u32 radius, u32 color) {
    k_fill_rect(x, y, w, h, color);
}

void k_draw_circle(u32 cx, u32 cy, u32 r, u32 color) {
    (void)cx; (void)cy; (void)r; (void)color;
}

void k_fill_circle(u32 cx, u32 cy, u32 r, u32 color) {
    (void)cx; (void)cy; (void)r; (void)color;
}

void k_draw_char(u32 x, u32 y, char c, u32 color, u32 scale) {
    (void)scale;
    enum vga_color fg = (enum vga_color)(color & 0xF);
    enum vga_color bg = (enum vga_color)((color >> 4) & 0xF);
    u8 attr = vga_entry_color(fg, bg);
    if (x < VGA_WIDTH && y < VGA_HEIGHT)
        vga_put_at(x, y, c, attr);
}

void k_draw_text(u32 x, u32 y, const char *s, u32 color, u32 scale) {
    (void)scale;
    enum vga_color fg = (enum vga_color)(color & 0xF);
    enum vga_color bg = (enum vga_color)((color >> 4) & 0xF);
    u8 attr = vga_entry_color(fg, bg);
    u32 cx = x;
    while (*s && cx < VGA_WIDTH) {
        if (*s == '\n') { cx = x; y++; if (y >= VGA_HEIGHT) break; }
        else { vga_put_at(cx, y, *s, attr); cx++; }
        s++;
    }
}

u32 k_text_width(const char *s, u32 scale) {
    (void)scale;
    u32 w = 0;
    while (*s) { if (*s != '\n') w++; s++; }
    return w;
}

void k_gradient_rect(u32 x, u32 y, u32 w, u32 h, u32 c1, u32 c2, int vertical) {
    (void)vertical;
    k_fill_rect(x, y, w, h, c1);
}

/* Simple integer square root (used by aarch64, defined here too for completeness) */
static u32 k_sqrt(u32 n) {
    if (n == 0) return 0;
    u32 x = n;
    u32 y = (x + 1) / 2;
    while (y < x) {
        x = y;
        y = (x + n / x) / 2;
    }
    return x;
}
#endif /* __aarch64__ / __x86_64__ */
