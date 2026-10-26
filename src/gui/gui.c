/* gui.c — KlikOS Mobile GUI subsystem glue. */

#include <kernel/fb.h>
#include <kernel/printf.h>
#include <kernel/types.h>
#include <gui/draw.h>
#include <gui/gui.h>
#include <gui/springboard.h>

int ui_state = UI_STATE_LOCK;
int selected_app = -1;

void gui_subsystem_init(void)
{
    kprintf("[gui] Initializing framebuffer...\n");
    fb_init();

    kprintf("[gui] Initializing mobile springboard...\n");
    ui_state = UI_STATE_LOCK;

    kprintf("[gui] GUI subsystem ready\n");
}

void gui_subsystem_draw(void)
{
    ui_draw_frame();
}

void gui_tap(u32 x, u32 y)
{
    (void)x; (void)y;
}

void gui_swipe(u32 from_x, u32 from_y, u32 to_x, u32 to_y)
{
    (void)from_x; (void)from_y; (void)to_x; (void)to_y;
}
