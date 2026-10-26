/* shell.c — KlikOS shell (desktop) with window manager and input handling.
 *
 * Provides: lock screen, home screen (springboard), app windows,
 * control center, settings, and input event routing.
 */

#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 40 app definitions */
static const char *app_names[40] = {
    "Settings", "Files", "Terminal", "Browser", "App Store",
    "WhatsApp", "Camera", "Gallery", "Phone", "Messages",
    "Email", "Calendar", "Clock", "Calculator", "Notes",
    "Reminders", "Maps", "Weather", "Music", "Podcasts",
    "Videos", "TV", "Books", "News", "Stocks",
    "Health", "Fitness", "Home", "Shortcuts", "Measure",
    "Translate", "Voice Memos", "Find My", "Contacts",
    "FaceTime", "Game Center", "TestFlight", "Tips",
    "Accessibility", "System Update"
};

static const char *app_glyphs[40] = {
    "GN","FL","TR","BR","AS","WA","CA","GA","PH","MG",
    "EM","CL","CK","CC","NT","RM","MP","WE","MU","PC",
    "VI","TV","BK","NW","SK","HL","FT","HM","SC","MS",
    "TL","VM","FM","CT","FT","GC","TF","TP","AC","SU"
};

static const char *app_content[40];
/* Helper: parse hex color */
k_color_t k_color_from_hex(uint32_t hex) {
    k_color_t c;
    c.a = (hex >> 24) & 0xFF;
    c.r = (hex >> 16) & 0xFF;
    c.g = (hex >> 8) & 0xFF;
    c.b = hex & 0xFF;
    return c;
}

static k_color_t app_colors[40];

static void draw_lock_screen(k_shell_t *shell) {
    k_display_t *d = shell->disp;
    k_color_t bg = k_color(0, 0, 46, 255);
    k_draw_fill_rect(d, (k_rect_t){0, 0, d->width, d->height}, bg);

    k_draw_text(d, d->width/2 - 120, d->height/2 - 60, "12:00",
                k_color(255,255,255,255), 4);
    k_draw_text(d, d->width/2 - 100, d->height/2 + 30, "Sun, Oct 25",
                k_color(255,255,255,255), 1);

    k_draw_fill_circle(d, d->width/2, d->height - 120, 8, k_color(255,255,255,255));
    k_draw_text(d, d->width/2 - 60, d->height - 100, "Swipe up to unlock",
                k_color(255,255,255,255), 1);

    k_draw_fill_circle(d, d->width/4, d->height - 80, 24, k_color(51,51,51,255));
    k_draw_text(d, d->width/4 - 12, d->height - 75, "CAM", k_color(255,255,255,255), 1);
    k_draw_fill_circle(d, 3*d->width/4, d->height - 80, 24, k_color(51,51,51,255));
    k_draw_text(d, 3*d->width/4 - 16, d->height - 75, "LIGHT", k_color(255,255,255,255), 1);
}

/* Home screen (Springboard) */
static void draw_home_screen(k_shell_t *shell) {
    k_display_t *d = shell->disp;
    k_draw_fill_rect(d, (k_rect_t){0, 0, d->width, d->height},
                     k_color(242, 242, 247, 255));

    /* Status bar */
    k_draw_fill_rect(d, (k_rect_t){0, 0, d->width, 44}, k_color(242,242,247,255));
    k_draw_text(d, 20, 14, "KlikOS", k_color(0,0,0,255), 1);
    k_draw_text(d, d->width - 120, 14, "12:00  100%", k_color(0,0,0,255), 1);

    /* App grid: 4 columns */
    int col_w = d->width / 4;
    int row_h = 110;
    int start_y = 90;
    int icon_size = 60;

    for (int i = 0; i < 40; i++) {
        int col = i % 4;
        int row = i / 4;
        int x = col * col_w + (col_w - icon_size) / 2;
        int y = start_y + row * row_h + 10;

        k_draw_fill_round_rect(d, (k_rect_t){x, y, icon_size, icon_size}, icon_size/5,
                               app_colors[i]);
        k_draw_fill_round_rect(d, (k_rect_t){x+4, y+4, icon_size-8, icon_size/2}, icon_size/6,
                               k_color(255,255,255,255));
        int gw = k_text_width(app_glyphs[i], 2);
        k_draw_text(d, x + (icon_size - gw)/2, y + icon_size/2 - 7,
                    app_glyphs[i], k_color(255,255,255,255), 2);
        int lw = k_text_width(app_names[i], 1);
        k_draw_text(d, x + (icon_size - lw)/2, y + icon_size + 4,
                    app_names[i], k_color(0,0,0,255), 1);
    }

    /* Dock */
    int dock_y = d->height - 110;
    k_draw_fill_round_rect(d, (k_rect_t){20, dock_y, d->width - 40, 70}, 22,
                           k_color(255,255,255,255));

    const char *dock_names[4] = {"Phone", "Messages", "Browser", "App Store"};
    const char *dock_glyphs[4] = {"PH","MG","BR","AS"};
    uint32_t dock_colors[4] = {0xFF007AFF, 0xFF007AFF, 0xFF007AFF, 0xFF007AFF};
    int dock_icon = 50;
    int dock_spacing = (d->width - 40 - 4 * dock_icon) / 5;

    for (int i = 0; i < 4; i++) {
        int x = 20 + dock_spacing + i * (dock_icon + dock_spacing);
        k_draw_fill_round_rect(d, (k_rect_t){x, dock_y + 10, dock_icon, dock_icon}, dock_icon/5,
                               k_color_from_hex(dock_colors[i]));
        int gw = k_text_width(dock_glyphs[i], 2);
        k_draw_text(d, x + (dock_icon - gw)/2, dock_y + 10 + dock_icon/2 - 7,
                    dock_glyphs[i], k_color(255,255,255,255), 2);
        int lw = k_text_width(dock_names[i], 1);
        k_draw_text(d, x + (dock_icon - lw)/2, dock_y + 10 + dock_icon + 4,
                    dock_names[i], k_color(0,0,0,255), 1);
    }

    k_draw_text(d, d->width/2 - 10, d->height - 40, "1  2", k_color(142,142,147,255), 1);
}

/* Control Center */
static void draw_control_center(k_shell_t *shell) {
    k_display_t *d = shell->disp;
    k_draw_fill_rect(d, (k_rect_t){0, 0, d->width, d->height}, k_color(26,26,26,255));

    k_draw_fill_rect(d, (k_rect_t){d->width/2 - 35, 15, 70, 6}, k_color(68,68,68,255));
    k_draw_text(d, d->width/2 - 70, 50, "Control Center", k_color(255,255,255,255), 1);

    int items_w = 80, items_h = 80, gap = 30;
    int start_x = (d->width - (4 * items_w + 3 * gap)) / 2;
    int start_y = 120;

    struct { uint32_t color; const char *glyph; const char *label; } items[4] = {
        {0xFF007AFF, "AF", "Airplane"},
        {0xFF8E8E93, "WI", "Wi-Fi"},
        {0xFF8E8E93, "BT", "BT"},
        {0xFF34C759, "CE", "Cell"}
    };

    for (int i = 0; i < 4; i++) {
        int x = start_x + i * (items_w + gap);
        k_draw_fill_round_rect(d, (k_rect_t){x, start_y, items_w, items_h}, 16,
                               k_color_from_hex(items[i].color));
        int gw = k_text_width(items[i].glyph, 2);
        k_draw_text(d, x + items_w/2 - gw/2, start_y + 24, items[i].glyph,
                    k_color(255,255,255,255), 2);
        int lw = k_text_width(items[i].label, 1);
        k_draw_text(d, x + items_w/2 - lw/2, start_y + 60, items[i].label,
                    k_color(255,255,255,255), 1);
    }

    k_draw_text(d, 40, start_y + 130, "Brightness", k_color(255,255,255,255), 1);
    k_draw_fill_rect(d, (k_rect_t){40, start_y + 150, 200, 6}, k_color(68,68,68,255));
    k_draw_fill_rect(d, (k_rect_t){40, start_y + 150, 150, 6}, k_color(255,255,255,255));
    k_draw_fill_circle(d, 190, start_y + 153, 10, k_color(255,255,255,255));

    k_draw_text(d, 40, start_y + 190, "Volume", k_color(255,255,255,255), 1);
    k_draw_fill_rect(d, (k_rect_t){40, start_y + 210, 200, 6}, k_color(68,68,68,255));
    k_draw_fill_rect(d, (k_rect_t){40, start_y + 210, 100, 6}, k_color(255,255,255,255));
    k_draw_fill_circle(d, 140, start_y + 213, 10, k_color(255,255,255,255));

    k_draw_text(d, d->width/2 - 40, d->height - 40, "Swipe up to close",
                k_color(142,142,147,255), 1);
}

/* Settings */
static int toggle_wifi = 1;
static int toggle_bluetooth = 0;
static int toggle_dark = 0;

static void draw_settings(k_shell_t *shell) {
    k_display_t *d = shell->disp;
    k_draw_fill_rect(d, (k_rect_t){0, 0, d->width, d->height}, k_color(255,255,255,255));

    k_draw_fill_rect(d, (k_rect_t){0, 0, d->width, 90}, k_color(255,255,255,255));
    k_draw_text(d, 30, 30, "Settings", k_color(0,0,0,255), 2);

    int y = 110;
    int row_h = 70;

    #define SETTINGS_ROW(title, toggle_val) \
        k_draw_fill_rect(d, (k_rect_t){30, y, d->width - 60, row_h}, k_color(255,255,255,255)); \
        k_draw_text(d, 50, y + 12, title, k_color(0,0,0,255), 1); \
        k_draw_fill_round_rect(d, (k_rect_t){d->width - 80, y + 15, 50, 28}, 14, \
            toggle_val ? k_color(0,122,255,255) : k_color(198,198,200,255)); \
        k_draw_fill_circle(d, toggle_val ? d->width - 46 : d->width - 66, y + 29, 10, \
            k_color(255,255,255,255)); \
        y += row_h;

    SETTINGS_ROW("Airplane Mode", toggle_wifi);
    SETTINGS_ROW("Wi-Fi", toggle_wifi);
    SETTINGS_ROW("Bluetooth", toggle_bluetooth);
    SETTINGS_ROW("Dark Mode", toggle_dark);
    SETTINGS_ROW("Display & Brightness", 0);
    SETTINGS_ROW("Sounds & Haptics", 0);
    SETTINGS_ROW("Wallpaper", 0);
    SETTINGS_ROW("General", 0);
    SETTINGS_ROW("About", 0);

    k_draw_text(d, d->width/2 - 50, d->height - 40, "Swipe right to go back",
                k_color(142,142,147,255), 1);
    #undef SETTINGS_ROW
}

/* App window */
static void draw_app_window(k_shell_t *shell, const char *title, k_color_t accent,
                            const char *content) {
    k_display_t *d = shell->disp;
    k_draw_fill_rect(d, (k_rect_t){0, 0, d->width, d->height}, k_color(242,242,247,255));

    k_draw_fill_rect(d, (k_rect_t){0, 0, d->width, 90}, accent);
    k_draw_text(d, 30, 30, title, k_color(255,255,255,255), 2);
    k_draw_text(d, 20, 55, "< Back", k_color(255,255,255,255), 1);

    k_draw_text(d, 30, 130, content, k_color(0,0,0,255), 1);

    k_draw_fill_rect(d, (k_rect_t){d->width/2 - 60, d->height - 30, 120, 6},
                     k_color(198,198,200,255));
    k_draw_fill_round_rect(d, (k_rect_t){d->width/2 - 10, d->height - 24, 20, 4}, 2,
                           k_color(142,142,147,255));
}

/* Shell initialization */
void k_shell_init(k_shell_t *shell, k_display_t *disp) {
    shell->state = SHELL_STATE_LOCK;
    shell->disp = disp;
    shell->screen_width = disp->width;
    shell->screen_height = disp->height;
    wm_init(&shell->wm);

    /* Initialize app colors */
    static const uint32_t app_colors_hex[40] = {
        0xFF007AFF, 0xFF34C759, 0xFF000000, 0xFF007AFF, 0xFF007AFF,
        0xFF007AFF, 0xFF007AFF, 0xFFFF2D55, 0xFF007AFF, 0xFF007AFF,
        0xFF007AFF, 0xFF34C759, 0xFFFF9500, 0xFF007AFF, 0xFFFFCC00,
        0xFFFF9500, 0xFF007AFF, 0xFFFF9500, 0xFF007AFF, 0xFFAF52DE,
        0xFFFF2D55, 0xFFFF9500, 0xFF007AFF, 0xFFFF2D55, 0xFF007AFF,
        0xFF34C759, 0xFFFF9500, 0xFF007AFF, 0xFFFF2D55, 0xFF007AFF,
        0xFFAF52DE, 0xFFFFCC00, 0xFF007AFF, 0xFF007AFF, 0xFFFF2D55,
        0xFF34C759, 0xFFFF9500, 0xFF007AFF
    };
    for (int i = 0; i < 40; i++) {
        app_colors[i] = k_color_from_hex(app_colors_hex[i]);
    }

    printf("[shell] Shell initialized\n");
}

/* Main draw */
void k_shell_draw(k_shell_t *shell) {
    if (!shell || !shell->disp) return;

    switch (shell->state) {
        case SHELL_STATE_LOCK:
            draw_lock_screen(shell);
            break;
        case SHELL_STATE_HOME:
            wm_draw_all(&shell->wm, shell->disp);
            break;
        case SHELL_STATE_CONTROL:
            draw_control_center(shell);
            break;
        case SHELL_STATE_SETTINGS:
            draw_settings(shell);
            break;
        case SHELL_STATE_APP:
        {
            int idx = shell->app_index;
            if (idx < 0 || idx >= 40) idx = 0;
            draw_app_window(shell, app_names[idx], app_colors[idx],
                            app_content[idx]);
            break;
        }
        default:
            wm_draw_all(&shell->wm, shell->disp);
            break;
    }
}

/* Shell input handling */
static void app_launch(k_shell_t *shell, int index);

void k_shell_tap(k_shell_t *shell, int x, int y) {
    if (!shell) return;

    if (shell->state == SHELL_STATE_LOCK) {
        shell->state = SHELL_STATE_HOME;
        return;
    }

    if (shell->state == SHELL_STATE_HOME) {
        int dock_y = shell->disp->height - 110;
        if (y > dock_y && y < dock_y + 70) {
            int dock_spacing = (shell->disp->width - 40 - 4 * 50) / 5;
            for (int i = 0; i < 4; i++) {
                int dx = 20 + dock_spacing + i * (50 + dock_spacing);
                if (x > dx && x < dx + 50 && y > dock_y + 10 && y < dock_y + 60) {
                    int idx = (i == 0) ? 8 : (i == 1) ? 9 : (i == 2) ? 3 : 4;
                    app_launch(shell, idx);
                    return;
                }
            }
        }

        int col_w = shell->disp->width / 4;
        int row_h = 110;
        int start_y = 90;
        if (y > start_y && y < shell->disp->height - 110) {
            int col = x / col_w;
            int row = (y - start_y) / row_h;
            int idx = row * 4 + col;
            if (idx >= 0 && idx < 40) {
                app_launch(shell, idx);
                return;
            }
        }

        if (y < 40 && x > shell->disp->width * 3 / 4) {
            shell->state = SHELL_STATE_CONTROL;
            return;
        }
    }

    if (shell->state == SHELL_STATE_APP) {
        if (y > shell->disp->height - 40) {
            shell->state = SHELL_STATE_HOME;
            wm_destroy_window(&shell->wm, shell->wm.focused_window);
            return;
        }
    }

    if (shell->state == SHELL_STATE_CONTROL) {
        if (y > shell->disp->height - 60) {
            shell->state = SHELL_STATE_HOME;
        }
    }

    if (shell->state == SHELL_STATE_SETTINGS) {
        if (y > shell->disp->height - 60) {
            shell->state = SHELL_STATE_HOME;
        }
    }
}

/* Process input events */
void k_shell_handle_event(k_shell_t *shell, klikos_event_t *ev) {
    if (!shell || !ev) return;

    switch (ev->type) {
    case KLIKOS_EVENT_MOUSEDOWN:
        if (ev->code == 1) {
            shell->wm.mouse_x = ev->x;
            shell->wm.mouse_y = ev->y;
            wm_handle_event(&shell->wm, shell->disp, ev);
        }
        break;
    case KLIKOS_EVENT_MOUSEMOVE:
        shell->wm.mouse_x = ev->x;
        shell->wm.mouse_y = ev->y;
        wm_handle_event(&shell->wm, shell->disp, ev);
        break;
    case KLIKOS_EVENT_MOUSEUP:
        if (ev->code == 1) {
            wm_handle_event(&shell->wm, shell->disp, ev);
        }
        break;
    case KLIKOS_EVENT_KEYDOWN:
        wm_handle_event(&shell->wm, shell->disp, ev);
        break;
    case KLIKOS_EVENT_KEYUP:
        wm_handle_event(&shell->wm, shell->disp, ev);
        break;
    default:
        break;
    }
}

void k_shell_cleanup(k_shell_t *shell) {
    wm_cleanup(&shell->wm);
    printf("[shell] Shell cleaned up\n");
}


/* App content descriptions (file scope so k_shell_draw can use them) */
static const char *app_content[40] = {
    "Configure your device settings.",
    "Browse your files and folders.",
    "Command line interface.",
    "Browse the web.",
    "Download apps and games.",
    "Message your friends.",
    "Take photos and videos.",
    "View your photos.",
    "Make phone calls.",
    "Send text messages.",
    "Send and receive email.",
    "Manage your calendar.",
    "View time and alarms.",
    "Perform calculations.",
    "Take notes.",
    "Set reminders.",
    "View maps and navigate.",
    "Check the weather.",
    "Listen to music.",
    "Listen to podcasts.",
    "Watch videos.",
    "Watch TV.",
    "Read books.",
    "Read news.",
    "Check stocks.",
    "Track health.",
    "Track fitness.",
    "Control home devices.",
    "Create shortcuts.",
    "Measure distances.",
    "Translate text.",
    "Record voice memos.",
    "Find devices.",
    "Manage contacts.",
    "Video call.",
    "Play games.",
    "Test apps.",
    "Get tips.",
    "Accessibility options.",
    "Update system."
};

static void app_launch(k_shell_t *shell, int index)
{
    if (index < 0 || index >= 40) return;
    shell->state   = SHELL_STATE_APP;
    shell->app_index = index;
    printf("[shell] Launching app %d: %s\n", index, app_names[index]);
}
