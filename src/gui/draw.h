#ifndef KLIKOS_GUI_DRAW_H
#define KLIKOS_GUI_DRAW_H

#include <kernel/types.h>

/* iOS-style colors */
#define K_COLOR_SYSTEM_BG      0xF2F2F7
#define K_COLOR_SYSTEM_GROUPED 0xFFFFFF
#define K_COLOR_LABEL          0x000000
#define K_COLOR_SECONDARY      0x3C3C43
#define K_COLOR_TERTIARY       0xAFB1C0
#define K_COLOR_BLUE           0x007AFF
#define K_COLOR_GREEN          0x34C759
#define K_COLOR_RED            0xFF3B30
#define K_COLOR_ORANGE         0xFF9500
#define K_COLOR_GRAY           0x8E8E93
#define K_COLOR_SEPARATOR      0xC6C6C8
#define K_COLOR_CONTROL_BG     0xE5E5EA

/* Drawing primitives */
void k_draw_rect(u32 x, u32 y, u32 w, u32 h, u32 color);
void k_fill_rect(u32 x, u32 y, u32 w, u32 h, u32 color);
void k_fill_round_rect(u32 x, u32 y, u32 w, u32 h, u32 radius, u32 color);
void k_draw_circle(u32 cx, u32 cy, u32 r, u32 color);
void k_fill_circle(u32 cx, u32 cy, u32 r, u32 color);
void k_draw_line(u32 x1, u32 y1, u32 x2, u32 y2, u32 color);
void k_clear(u32 color);
void k_gradient_rect(u32 x, u32 y, u32 w, u32 h, u32 c1, u32 c2, int vertical);

/* Text - simple bitmap font rendering */
void k_draw_char(u32 x, u32 y, char c, u32 color, u32 scale);
void k_draw_text(u32 x, u32 y, const char *s, u32 color, u32 scale);
u32  k_text_width(const char *s, u32 scale);

#endif /* KLIKOS_GUI_DRAW_H */
