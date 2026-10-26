#ifndef KLIKOS_SPRINGBOARD_H
#define KLIKOS_SPRINGBOARD_H

#include <kernel/types.h>

void springboard_init(void);
void springboard_draw(void);
void springboard_tap(u32 x, u32 y);
void springboard_swipe(u32 from_x, u32 from_y, u32 to_x, u32 to_y);
void ui_draw_frame(void);
void app_launch(int index);
void settings_draw(void);
void control_center_draw(void);
void lock_screen_draw(void);

#endif /* KLIKOS_SPRINGBOARD_H */
