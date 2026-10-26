/* vga.c — VGA text mode driver for x86_64.
 *
 * Writes directly to the VGA text buffer at 0xB8000.
 * Provides basic console output before the GUI is ready.
 */

#include <kernel/vga.h>
#include <kernel/types.h>

static u16 *vga_buffer = (u16 *)VGA_BASE;
static u32 vga_row = 0;
static u32 vga_col = 0;
static u8 vga_color = VGA_COLOR_LIGHT_GREY | VGA_COLOR_BLACK << 4;

static void vga_scroll(void) {
    for (u32 y = 0; y < VGA_HEIGHT - 1; y++) {
        for (u32 x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[y * VGA_WIDTH + x] = vga_buffer[(y + 1) * VGA_WIDTH + x];
        }
    }
    for (u32 x = 0; x < VGA_WIDTH; x++) {
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', vga_color);
    }
    vga_row = VGA_HEIGHT - 1;
}

void vga_init(void) {
    vga_clear();
}

void vga_put_at(u32 x, u32 y, char c, u8 color) {
    vga_buffer[y * VGA_WIDTH + x] = vga_entry((unsigned char)c, color);
}

void vga_putc(char c) {
    if (c == '\n') {
        vga_col = 0;
        vga_row++;
    } else {
        vga_put_at(vga_col, vga_row, c, vga_color);
        vga_col++;
        if (vga_col >= VGA_WIDTH) {
            vga_col = 0;
            vga_row++;
        }
    }
    if (vga_row >= VGA_HEIGHT) {
        vga_scroll();
    }
}

void vga_puts(const char *s) {
    while (*s) {
        vga_putc(*s++);
    }
}

void vga_clear(void) {
    for (u32 y = 0; y < VGA_HEIGHT; y++) {
        for (u32 x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[y * VGA_WIDTH + x] = vga_entry(' ', vga_color);
        }
    }
    vga_row = 0;
    vga_col = 0;
}
