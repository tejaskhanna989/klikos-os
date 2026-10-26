#ifndef KLIKOS_TOOLKIT_H
#define KLIKOS_TOOLKIT_H

#include "display.h"
#include "window_manager.h"
#include <stdint.h>
#include <stddef.h>

/* Widget types */
typedef enum {
    WIDGET_WINDOW,
    WIDGET_BUTTON,
    WIDGET_LABEL,
    WIDGET_TEXT_FIELD,
    WIDGET_TOGGLE,
    WIDGET_SLIDER,
    WIDGET_IMAGE,
    WIDGET_LIST_ITEM
} widget_type_t;

/* Widget base */
typedef struct k_widget k_widget_t;

struct k_widget {
    widget_type_t type;
    k_rect_t frame;
    k_color_t bg_color;
    k_color_t fg_color;
    int visible;
    int enabled;
    int focused;
    void *user_data;
    void (*draw)(k_widget_t *w, k_display_t *disp);
    void (*on_tap)(k_widget_t *w);
    void (*on_touch)(k_widget_t *w, int x, int y, int down);
    struct k_widget *parent;
    struct k_widget *first_child;
    struct k_widget *next_sibling;
};

/* Button */
typedef struct {
    k_widget_t base;
    char label[64];
    void (*on_click)(k_widget_t *w);
} k_button_t;

/* Label */
typedef struct {
    k_widget_t base;
    char text[128];
    int scale;
} k_label_t;

/* Toggle switch */
typedef struct {
    k_widget_t base;
    int value;
    void (*on_change)(k_widget_t *w, int value);
} k_toggle_t;

/* Create/destroy */
k_widget_t *k_widget_create(widget_type_t type, k_rect_t frame);
void k_widget_destroy(k_widget_t *w);
void k_widget_add_child(k_widget_t *parent, k_widget_t *child);
void k_widget_remove_child(k_widget_t *parent, k_widget_t *child);

/* Draw/layout */
void k_widget_draw(k_widget_t *w, k_display_t *disp);
void k_widget_draw_tree(k_widget_t *root, k_display_t *disp);
int  k_widget_hit_test(k_widget_t *w, int x, int y);

/* Convenience constructors */
k_button_t *k_button_create(const char *label, k_rect_t frame, void (*on_click)(k_widget_t *w));
k_label_t *k_label_create(const char *text, k_rect_t frame, int scale);
k_toggle_t *k_toggle_create(k_rect_t frame, int value, void (*on_change)(k_widget_t *w, int value));

/* Default colors */
#define K_COLOR_SYSTEM_BG      k_color(242, 242, 247, 255)
#define K_COLOR_SYSTEM_GROUPED k_color(255, 255, 255, 255)
#define K_COLOR_LABEL          k_color(0, 0, 0, 255)
#define K_COLOR_SECONDARY      k_color(60, 60, 67, 255)
#define K_COLOR_TERTIARY       k_color(174, 177, 192, 255)
#define K_COLOR_BLUE           k_color(0, 122, 255, 255)
#define K_COLOR_GREEN          k_color(52, 199, 89, 255)
#define K_COLOR_RED            k_color(255, 59, 48, 255)
#define K_COLOR_ORANGE         k_color(255, 149, 0, 255)
#define K_COLOR_GRAY           k_color(142, 142, 147, 255)
#define K_COLOR_SEPARATOR      k_color(198, 198, 200, 255)
#define K_COLOR_CONTROL_BG     k_color(229, 229, 234, 255)

#endif /* KLIKOS_TOOLKIT_H */
