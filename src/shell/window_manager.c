/* window_manager.c — KlikOS Window Manager.
 *
 * Handles multiple windows with z-order, focus, dragging, and input dispatch.
 */

#include "window_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TITLE_BAR_HEIGHT 28
#define BORDER_WIDTH 2
#define SCROLLBAR_WIDTH 16

static int next_window_id = 1;

/* Sort windows by z-order (lowest first, so they draw bottom-to-top) */
static int wm_compare_zorder(const void *a, const void *b) {
    const k_window_t *wa = (const k_window_t *)a;
    const k_window_t *wb = (const k_window_t *)b;
    return (int)wa->z_order - (int)wb->z_order;
}

void wm_init(k_wm_t *wm) {
    memset(wm, 0, sizeof(*wm));
    wm->window_count = 0;
    wm->focused_window = -1;
    wm->focus_policy = WM_FOCUS_CLICK;
    wm->mouse_x = 0;
    wm->mouse_y = 0;
    wm->drag_window = -1;
    wm->drag_offset_x = 0;
    wm->drag_offset_y = 0;
    printf("[wm] Window manager initialized\n");
}

int wm_create_window(k_wm_t *wm, const char *title, k_rect_t frame,
                     k_color_t bg, k_color_t title_color,
                     void (*draw)(k_window_t *, k_display_t *)) {
    if (wm->window_count >= MAX_WINDOWS) {
        fprintf(stderr, "[wm] Maximum windows reached\n");
        return -1;
    }

    k_window_t *win = &wm->windows[wm->window_count];
    memset(win, 0, sizeof(*win));

    win->id = next_window_id++;
    strncpy(win->title, title, sizeof(win->title) - 1);
    win->title[sizeof(win->title) - 1] = '\0';
    win->frame = frame;
    win->bg_color = bg;
    win->title_color = title_color;
    win->border_color = k_color(100, 100, 100, 255);
    win->state = WM_STATE_NORMAL;
    win->focused = 0;
    win->visible = 1;
    win->z_order = wm->window_count;  /* insertion order */
    win->draw = draw;

    /* Client area inside title bar and borders */
    win->client.x = frame.x + BORDER_WIDTH;
    win->client.y = frame.y + TITLE_BAR_HEIGHT + BORDER_WIDTH;
    win->client.w = frame.w - 2 * BORDER_WIDTH;
    win->client.h = frame.h - TITLE_BAR_HEIGHT - 2 * BORDER_WIDTH;

    wm->windows[wm->window_count++] = *win;
    int id = win->id;

    /* Focus the new window */
    wm_set_focus(wm, id);

    printf("[wm] Window created: %s (id=%d)\n", title, id);
    return id;
}

void wm_destroy_window(k_wm_t *wm, int id) {
    int idx = -1;
    for (int i = 0; i < wm->window_count; i++) {
        if (wm->windows[i].id == id) {
            idx = i;
            break;
        }
    }
    if (idx < 0) return;

    /* Remove window */
    for (int i = idx; i < wm->window_count - 1; i++) {
        wm->windows[i] = wm->windows[i + 1];
    }
    wm->window_count--;

    /* If focused window was destroyed, unfocus or focus next */
    if (wm->focused_window == id) {
        wm->focused_window = -1;
        if (wm->window_count > 0) {
            wm_set_focus(wm, wm->windows[wm->window_count - 1].id);
        }
    }

    /* Update z-orders */
    for (int i = 0; i < wm->window_count; i++) {
        wm->windows[i].z_order = i;
    }

    printf("[wm] Window destroyed: id=%d\n", id);
}

void wm_set_focus(k_wm_t *wm, int id) {
    if (wm->focused_window == id) return;

    /* Unfocus current */
    if (wm->focused_window >= 0) {
        for (int i = 0; i < wm->window_count; i++) {
            if (wm->windows[i].id == wm->focused_window) {
                wm->windows[i].focused = 0;
                break;
            }
        }
    }

    wm->focused_window = id;

    /* Focus new */
    if (id >= 0) {
        for (int i = 0; i < wm->window_count; i++) {
            if (wm->windows[i].id == id) {
                wm->windows[i].focused = 1;
                wm_bring_to_front(wm, id);
                break;
            }
        }
    }
}

int wm_get_focused(k_wm_t *wm) {
    return wm->focused_window;
}

void wm_bring_to_front(k_wm_t *wm, int id) {
    int idx = -1;
    for (int i = 0; i < wm->window_count; i++) {
        if (wm->windows[i].id == id) {
            idx = i;
            break;
        }
    }
    if (idx < 0) return;

    /* Move to end (highest z-order) */
    k_window_t win = wm->windows[idx];
    for (int i = idx; i < wm->window_count - 1; i++) {
        wm->windows[i] = wm->windows[i + 1];
    }
    wm->windows[wm->window_count - 1] = win;

    /* Renumber z-orders */
    for (int i = 0; i < wm->window_count; i++) {
        wm->windows[i].z_order = i;
    }
}

int wm_point_in_window(k_window_t *win, int x, int y) {
    return (x >= win->frame.x && x < win->frame.x + win->frame.w &&
            y >= win->frame.y && y < win->frame.y + win->frame.h);
}

int wm_point_in_titlebar(k_window_t *win, int x, int y) {
    return (x >= win->frame.x && x < win->frame.x + win->frame.w &&
            y >= win->frame.y && y < win->frame.y + TITLE_BAR_HEIGHT);
}

void wm_handle_event(k_wm_t *wm, k_display_t *disp, klikos_event_t *ev) {
    if (!wm || !ev) return;

    switch (ev->type) {
    case KLIKOS_EVENT_MOUSEMOVE:
        wm->mouse_x = ev->x;
        wm->mouse_y = ev->y;

        if (wm->drag_window >= 0) {
            /* Drag window */
            for (int i = 0; i < wm->window_count; i++) {
                if (wm->windows[i].id == wm->drag_window) {
                    wm->windows[i].frame.x = ev->x - wm->drag_offset_x;
                    wm->windows[i].frame.y = ev->y - wm->drag_offset_y;
                    wm->windows[i].client.x = wm->windows[i].frame.x + BORDER_WIDTH;
                    wm->windows[i].client.y = wm->windows[i].frame.y + TITLE_BAR_HEIGHT + BORDER_WIDTH;
                    wm_bring_to_front(wm, wm->drag_window);
                    break;
                }
            }
        }
        break;

    case KLIKOS_EVENT_MOUSEDOWN:
        if (ev->code == 1) {  /* Left button */
            /* Find window under cursor (topmost first) */
            int clicked = -1;
            for (int i = wm->window_count - 1; i >= 0; i--) {
                if (wm->windows[i].visible &&
                    wm_point_in_window(&wm->windows[i], ev->x, ev->y)) {
                    clicked = i;
                    break;
                }
            }

            if (clicked >= 0) {
                k_window_t *win = &wm->windows[clicked];

                /* Check titlebar click for drag */
                if (wm_point_in_titlebar(win, ev->x, ev->y)) {
                    wm->drag_window = win->id;
                    wm->drag_offset_x = ev->x - win->frame.x;
                    wm->drag_offset_y = ev->y - win->frame.y;
                    wm_set_focus(wm, win->id);
                } else {
                    wm_set_focus(wm, win->id);
                    /* Call window's click handler if in client area */
                    if (win->on_click) {
                        win->on_click(win, ev->x - win->client.x, ev->y - win->client.y);
                    }
                }
            } else {
                /* Clicked desktop - unfocus all */
                wm_set_focus(wm, -1);
            }
        }
        break;

    case KLIKOS_EVENT_MOUSEUP:
        if (ev->code == 1) {
            wm->drag_window = -1;
        }
        break;

    case KLIKOS_EVENT_KEYDOWN:
        if (wm->focused_window >= 0) {
            for (int i = 0; i < wm->window_count; i++) {
                if (wm->windows[i].id == wm->focused_window &&
                    wm->windows[i].on_key) {
                    wm->windows[i].on_key(&wm->windows[i], ev->code, 1);
                }
            }
        }
        break;

    case KLIKOS_EVENT_KEYUP:
        if (wm->focused_window >= 0) {
            for (int i = 0; i < wm->window_count; i++) {
                if (wm->windows[i].id == wm->focused_window &&
                    wm->windows[i].on_key) {
                    wm->windows[i].on_key(&wm->windows[i], ev->code, 0);
                }
            }
        }
        break;
    }
}

void wm_draw_all(k_wm_t *wm, k_display_t *disp) {
    if (!wm || !disp) return;

    /* Sort windows by z-order (lowest first) */
    k_window_t sorted[MAX_WINDOWS];
    for (int i = 0; i < wm->window_count; i++) {
        sorted[i] = wm->windows[i];
    }
    qsort(sorted, wm->window_count, sizeof(k_window_t), wm_compare_zorder);

    /* Draw each window */
    for (int i = 0; i < wm->window_count; i++) {
        k_window_t *win = &sorted[i];
        if (!win->visible) continue;

        k_rect_t r = win->frame;
        k_color_t border = win->focused ? k_color(0, 122, 255, 255) : win->border_color;
        k_color_t title_bg = win->focused ? k_color(0, 122, 255, 255) : win->title_color;

        /* Window shadow (simple) */
        k_draw_fill_rect(disp, (k_rect_t){r.x + 3, r.y + 3, r.w, r.h}, k_color(0, 0, 0, 80));

        /* Window background */
        k_draw_fill_rect(disp, r, win->bg_color);

        /* Border */
        k_draw_rect(disp, r, border);

        /* Title bar */
        k_draw_fill_rect(disp, (k_rect_t){r.x, r.y, r.w, TITLE_BAR_HEIGHT}, title_bg);

        /* Title text */
        k_draw_text(disp, r.x + 10, r.y + 8, win->title, k_color(255, 255, 255, 255), 1);

        /* Close button */
        k_rect_t close_btn = {r.x + r.w - 30, r.y + 4, 24, 16};
        k_draw_fill_rect(disp, close_btn, k_color(255, 59, 48, 255));
        k_draw_text(disp, close_btn.x + 6, close_btn.y + 2, "x", k_color(255, 255, 255, 255), 1);

        /* Draw window contents */
        if (win->draw) {
            win->draw(win, disp);
        }
    }
}

void wm_cleanup(k_wm_t *wm) {
    wm->window_count = 0;
    wm->focused_window = -1;
    printf("[wm] Window manager cleaned up\n");
}