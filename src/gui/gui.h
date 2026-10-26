/* gui.h — KlikOS Mobile GUI subsystem. */

#ifndef KLIKOS_GUI_H
#define KLIKOS_GUI_H

#include <kernel/types.h>

/* UI states */
#define UI_STATE_LOCK      0
#define UI_STATE_HOME      1
#define UI_STATE_APP       2
#define UI_STATE_CONTROL   3
#define UI_STATE_SETTINGS  4

extern int ui_state;
extern int selected_app;

void gui_subsystem_init(void);
void gui_subsystem_draw(void);
void gui_tap(u32 x, u32 y);
void gui_swipe(u32 from_x, u32 from_y, u32 to_x, u32 to_y);

#endif /* KLIKOS_GUI_H */
