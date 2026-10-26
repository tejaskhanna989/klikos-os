#ifndef KLIKOS_DISPLAY_H
#define KLIKOS_DISPLAY_H

#include <stdint.h>
#include <stddef.h>

/* Display surface */
typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t stride;
    uint32_t bpp;
    uint32_t format;   /* 0 = RGB565, 1 = XRGB8888 */
    void    *fb;       /* Framebuffer pointer */
    int     fd;        /* DRM device fd */
} k_display_t;

/* Color (ARGB8888) */
typedef struct {
    uint8_t a, r, g, b;
} k_color_t;

/* Point */
typedef struct {
    int32_t x, y;
} k_point_t;

/* Rectangle */
typedef struct {
    int32_t x, y;
    int32_t w, h;
} k_rect_t;

/* Display server functions */
int  k_display_init(k_display_t *disp, const char *device);
void k_display_cleanup(k_display_t *disp);
void k_display_clear(k_display_t *disp, k_color_t color);
void k_display_flip(k_display_t *disp);

/* Drawing primitives */
void k_draw_rect(k_display_t *disp, k_rect_t rect, k_color_t color);
void k_draw_fill_rect(k_display_t *disp, k_rect_t rect, k_color_t color);
void k_draw_round_rect(k_display_t *disp, k_rect_t rect, int radius, k_color_t color);
void k_draw_fill_round_rect(k_display_t *disp, k_rect_t rect, int radius, k_color_t color);
void k_draw_circle(k_display_t *disp, int cx, int cy, int r, k_color_t color);
void k_draw_fill_circle(k_display_t *disp, int cx, int cy, int r, k_color_t color);
void k_draw_line(k_display_t *disp, int x1, int y1, int x2, int y2, k_color_t color);
void k_draw_pixel(k_display_t *disp, int x, int y, k_color_t color);

/* Text rendering (bitmap font) */
void k_draw_char(k_display_t *disp, int x, int y, char c, k_color_t color, int scale);
void k_draw_text(k_display_t *disp, int x, int y, const char *text, k_color_t color, int scale);
int  k_text_width(const char *text, int scale);

/* Color helpers */
k_color_t k_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
k_color_t k_color_rgb(uint8_t r, uint8_t g, uint8_t b);
k_color_t k_color_blend(k_color_t src, k_color_t dst);

#endif /* KLIKOS_DISPLAY_H */
