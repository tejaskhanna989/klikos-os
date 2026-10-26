#ifndef KLIKOS_KERNEL_VGA_H
#define KLIKOS_KERNEL_VGA_H

#include <kernel/types.h>

#define VGA_WIDTH  80
#define VGA_HEIGHT 25
#define VGA_BASE   0xB8000

enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
};

static inline u8 vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

static inline u16 vga_entry(unsigned char uc, u8 color) {
    return (u16)uc | (u16)color << 8;
}

void vga_init(void);
void vga_putc(char c);
void vga_puts(const char *s);
void vga_clear(void);
void vga_put_at(u32 x, u32 y, char c, u8 color);

#endif /* KLIKOS_KERNEL_VGA_H */
