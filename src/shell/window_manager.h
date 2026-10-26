/* window_manager.h — KlikOS Window Manager.
 *
 * Manages multiple windows with z-order, focus, and input dispatch.
 */

#ifndef KLIKOS_WM_H
#define KLIKOS_WM_H

#include "display.h"
#include "input.h"
#include <stdint.h>

#define MAX_WINDOWS 32

/* Window states */
#define WM_STATE_NORMAL   0
#define WM_STATE_MINIMIZED 1
#define WM_STATE_MAXIMIZED 2

/* Window focus policy */
#define WM_FOCUS_CLICK    0
#define WM_FOCUS_SLOPPY   1

typedef struct k_window_t k_window_t;

typedef struct k_window_t {
    int id;
    char title[64];
    k_rect_t frame;           /* window position and size */
    k_rect_t client;          /* client area (inside decorations) */
    k_color_t bg_color;
    k_color_t title_color;
    k_color_t border_color;
    int state;                /* WM_STATE_* */
    int focused;              /* 1 if focused */
    int visible;              /* 1 if visible */
    int z_order;              /* z-index (higher = on top) */
    void (*draw)(k_window_t *win, k_display_t *disp);  /* custom draw */
    void (*on_key)(k_window_t *win, int key, int down); /* key handler */
    void (*on_click)(k_window_t *win, int x, int y);    /* click handler */
    void *user_data;          /* app-specific data */
} k_window_t;

/* Window manager state */
typedef struct {
    k_window_t windows[MAX_WINDOWS];
    int window_count;
    int focused_window;       /* index of focused window, -1 if none */
    int focus_policy;         /* WM_FOCUS_CLICK or WM_FOCUS_SLOPPY */
    int mouse_x, mouse_y;     /* current mouse position */
    int drag_window;          /* window being dragged (-1 if none) */
    int drag_offset_x, drag_offset_y;
} k_wm_t;

/* Window manager functions */
void wm_init(k_wm_t *wm);
int wm_create_window(k_wm_t *wm, const char *title, k_rect_t frame,
                     k_color_t bg, k_color_t title_color,
                     void (*draw)(k_window_t *, k_display_t *));
void wm_destroy_window(k_wm_t *wm, int id);
void wm_set_focus(k_wm_t *wm, int id);
int wm_get_focused(k_wm_t *wm);
void wm_bring_to_front(k_wm_t *wm, int id);
void wm_handle_event(k_wm_t *wm, k_display_t *disp, klikos_event_t *ev);
void wm_draw_all(k_wm_t *wm, k_display_t *disp);
void wm_cleanup(k_wm_t *wm);

/* Helper: check if point is inside window */
int wm_point_in_window(k_window_t *win, int x, int y);

/* Helper: check if point is in title bar */
int wm_point_in_titlebar(k_window_t *win, int x, int y);

#endif /* KLIKOS_WM_H */