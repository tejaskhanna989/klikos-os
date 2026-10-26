/* shell.h — KlikOS shell (desktop) with window manager support.
 */

#ifndef KLIKOS_SHELL_H
#define KLIKOS_SHELL_H

#include "display.h"
#include "window_manager.h"
#include "input.h"
#include <stdint.h>

/* UI states */
typedef enum {
    SHELL_STATE_LOCK,
    SHELL_STATE_HOME,
    SHELL_STATE_APP,
    SHELL_STATE_CONTROL,
    SHELL_STATE_SETTINGS
} shell_state_t;

/* Shell context */
typedef struct {
    shell_state_t state;
    k_display_t *disp;
    k_wm_t wm;
    int screen_width;
    int screen_height;
    int app_index;
} k_shell_t;

/* Shell functions */
void k_shell_init(k_shell_t *shell, k_display_t *disp);
void k_shell_draw(k_shell_t *shell);
void k_shell_handle_event(k_shell_t *shell, klikos_event_t *ev);
void k_shell_launch_app(k_shell_t *shell, int app_index);
void k_shell_cleanup(k_shell_t *shell);

k_color_t k_color_from_hex(uint32_t hex);

#endif /* KLIKOS_SHELL_H */