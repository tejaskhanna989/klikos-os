/* toolkit.c — KlikOS GUI toolkit implementation.
 *
 * Widget toolkit: windows, buttons, labels, toggles, sliders, drawing.
 */

#include "toolkit.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---------- Widget base ---------- */

k_widget_t *k_widget_create(widget_type_t type, k_rect_t frame) {
    k_widget_t *w = calloc(1, sizeof(k_widget_t));
    if (!w) return NULL;
    w->type = type;
    w->frame = frame;
    w->bg_color = k_color(255, 255, 255, 255);
    w->fg_color = k_color(0, 0, 0, 255);
    w->visible = 1;
    w->enabled = 1;
    w->focused = 0;
    w->user_data = NULL;
    w->draw = NULL;
    w->on_tap = NULL;
    w->on_touch = NULL;
    w->parent = NULL;
    w->first_child = NULL;
    w->next_sibling = NULL;
    return w;
}

void k_widget_destroy(k_widget_t *w) {
    if (!w) return;
    /* Remove children */
    k_widget_t *child = w->first_child;
    while (child) {
        k_widget_t *next = child->next_sibling;
        k_widget_destroy(child);
        child = next;
    }
    /* Remove from parent's sibling list */
    if (w->parent) {
        if (w->parent->first_child == w) {
            w->parent->first_child = w->next_sibling;
        } else {
            k_widget_t *s = w->parent->first_child;
            while (s && s->next_sibling != w) s = s->next_sibling;
            if (s) s->next_sibling = w->next_sibling;
        }
    }
    free(w);
}

void k_widget_add_child(k_widget_t *parent, k_widget_t *child) {
    if (!parent || !child) return;
    child->parent = parent;
    child->next_sibling = parent->first_child;
    parent->first_child = child;
}

void k_widget_remove_child(k_widget_t *parent, k_widget_t *child) {
    if (!parent || !child) return;
    if (parent->first_child == child) {
        parent->first_child = child->next_sibling;
    } else {
        k_widget_t *s = parent->first_child;
        while (s && s->next_sibling != child) s = s->next_sibling;
        if (s) s->next_sibling = child->next_sibling;
    }
    child->parent = NULL;
    child->next_sibling = NULL;
}

int k_widget_hit_test(k_widget_t *w, int x, int y) {
    if (!w || !w->visible || !w->enabled) return 0;
    return (x >= w->frame.x && x < w->frame.x + w->frame.w &&
            y >= w->frame.y && y < w->frame.y + w->frame.h);
}

/* ---------- Default draw functions ---------- */

static void button_draw(k_widget_t *w, k_display_t *disp) {
    k_button_t *btn = (k_button_t *)w;
    k_rect_t r = w->frame;
    k_color_t bg = w->focused ? k_color(0, 122, 255, 200) : k_color(0, 122, 255, 255);

    k_draw_fill_rect(disp, r, bg);
    k_draw_text(disp, r.x + (r.w - k_text_width(btn->label, 1)) / 2,
                r.y + (r.h - 7) / 2, btn->label, k_color(255, 255, 255, 255), 1);
}

static void label_draw(k_widget_t *w, k_display_t *disp) {
    k_label_t *lbl = (k_label_t *)w;
    k_draw_text(disp, w->frame.x, w->frame.y, lbl->text, w->fg_color, lbl->scale);
}

static void toggle_draw(k_widget_t *w, k_display_t *disp) {
    k_toggle_t *tgl = (k_toggle_t *)w;
    k_rect_t r = w->frame;
    k_color_t track = tgl->value ? k_color(52, 199, 89, 255) : k_color(142, 142, 147, 100);
    k_color_t thumb = k_color(255, 255, 255, 255);

    k_draw_round_rect(disp, r, r.h / 2, track);
    int thumb_d = r.h - 8;
    int thumb_x = tgl->value ? r.x + r.w - thumb_d - 4 : r.x + 4;
    k_draw_fill_circle(disp, thumb_x + thumb_d/2, r.y + r.h/2, thumb_d/2, thumb);
}

/* ---------- Widget draw dispatch ---------- */

void k_widget_draw(k_widget_t *w, k_display_t *disp) {
    if (!w || !w->visible) return;

    if (w->draw) {
        w->draw(w, disp);
    } else {
        switch (w->type) {
        case WIDGET_BUTTON:    button_draw(w, disp); break;
        case WIDGET_LABEL:     label_draw(w, disp); break;
        case WIDGET_TOGGLE:    toggle_draw(w, disp); break;
        default:
            k_draw_fill_rect(disp, w->frame, w->bg_color);
            break;
        }
    }

    /* Draw children */
    k_widget_t *child = w->first_child;
    while (child) {
        k_widget_draw(child, disp);
        child = child->next_sibling;
    }
}

void k_widget_draw_tree(k_widget_t *root, k_display_t *disp) {
    if (!root || !disp) return;
    k_widget_draw(root, disp);
}

/* ---------- Widget constructors ---------- */

k_button_t *k_button_create(const char *label, k_rect_t frame, void (*on_click)(k_widget_t *w)) {
    k_button_t *btn = calloc(1, sizeof(k_button_t));
    if (!btn) return NULL;
    k_widget_t *w = &btn->base;
    w->type = WIDGET_BUTTON;
    w->frame = frame;
    w->draw = button_draw;
    w->on_tap = on_click;
    strncpy(btn->label, label, sizeof(btn->label) - 1);
    btn->label[sizeof(btn->label) - 1] = '\0';
    btn->on_click = on_click;
    return btn;
}

k_label_t *k_label_create(const char *text, k_rect_t frame, int scale) {
    k_label_t *lbl = calloc(1, sizeof(k_label_t));
    if (!lbl) return NULL;
    k_widget_t *w = &lbl->base;
    w->type = WIDGET_LABEL;
    w->frame = frame;
    w->draw = label_draw;
    w->fg_color = K_COLOR_LABEL;
    strncpy(lbl->text, text, sizeof(lbl->text) - 1);
    lbl->text[sizeof(lbl->text) - 1] = '\0';
    lbl->scale = scale > 0 ? scale : 1;
    return lbl;
}

k_toggle_t *k_toggle_create(k_rect_t frame, int value, void (*on_change)(k_widget_t *w, int value)) {
    k_toggle_t *tgl = calloc(1, sizeof(k_toggle_t));
    if (!tgl) return NULL;
    k_widget_t *w = &tgl->base;
    w->type = WIDGET_TOGGLE;
    w->frame = frame;
    w->draw = toggle_draw;
    tgl->value = value ? 1 : 0;
    tgl->on_change = on_change;
    return tgl;
}
