/* springboard.c — iOS-style home screen with app grid, dock, control center,
 * lock screen, and settings.
 */

#include <kernel/fb.h>
#include <kernel/printf.h>
#include <kernel/string.h>
#include <gui/draw.h>
#include <gui/gui.h>
#include <gui/springboard.h>

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

static u32 app_colors[40] = {
    0x007AFF, 0x34C759, 0x000000, 0x007AFF, 0x007AFF,
    0x007AFF, 0x007AFF, 0xFF2D55, 0x007AFF, 0x007AFF,
    0x007AFF, 0x34C759, 0xFF9500, 0x007AFF, 0xFFCC00,
    0xFF9500, 0x007AFF, 0xFF9500, 0x007AFF, 0xAF52DE,
    0xFF2D55, 0xFF9500, 0x007AFF, 0xFF2D55, 0x007AFF,
    0x34C759, 0xFF9500, 0x007AFF, 0xFF2D55, 0x007AFF,
    0xAF52DE, 0xFFCC00, 0x007AFF, 0x007AFF, 0xFF2D55,
    0x34C759, 0xFF9500, 0x007AFF
};

static const char *app_glyphs[40] = {
    "GN", "FL", "TR", "BR", "AS",
    "WA", "CA", "GA", "PH", "MG",
    "EM", "CL", "CK", "CC", "NT",
    "RM", "MP", "WE", "MU", "PC",
    "VI", "TV", "BK", "NW", "SK",
    "HL", "FT", "HM", "SC", "MS",
    "TL", "VM", "FM", "CT", "FT",
    "GC", "TF", "TP", "AC", "SU"
};

/* --- App icon drawing --- */

static void draw_app_icon(u32 x, u32 y, u32 size, const char *label,
                          u32 color, const char *glyph)
{
    u32 icon_r = size / 6;
    (void)icon_r;

    /* Icon background (rounded square / squircle) */
    k_fill_round_rect(x, y, size, size, size / 5, color);

    /* Inner lighter highlight */
    k_fill_round_rect(x + 2, y + 2, size - 4, size / 2, size / 6, 0xFFFFFF);

    /* Glyph text (centered) */
    u32 gw = k_text_width(glyph, 2);
    u32 gx = x + (size - gw) / 2;
    u32 gy = y + size / 2 - 7;
    k_draw_text(gx, gy, glyph, 0xFFFFFF, 2);

    /* App label below */
    u32 lw = k_text_width(label, 1);
    u32 lx = x + (size - lw) / 2;
    u32 ly = y + size + 4;
    k_draw_text(lx, ly, label, 0x000000, 1);
}

/* --- Lock screen --- */

void lock_screen_draw(void)
{
    k_gradient_rect(0, 0, FB_WIDTH, FB_HEIGHT, 0x000000, 0x1A1A2E, 1);

    k_draw_text(FB_WIDTH / 2 - 60, FB_HEIGHT / 2 - 40, "12:00", 0xFFFFFF, 5);
    k_draw_text(FB_WIDTH / 2 - 50, FB_HEIGHT / 2 + 20, "Sun, Oct 25", 0xFFFFFF, 1);

    k_fill_circle(FB_WIDTH / 2, FB_HEIGHT - 100, 8, 0xFFFFFF);
    k_draw_text(FB_WIDTH / 2 - 30, FB_HEIGHT - 80, "Swipe up to unlock", 0xFFFFFF, 1);

    k_fill_circle(FB_WIDTH / 4, FB_HEIGHT - 60, 20, 0x333333);
    k_draw_text(FB_WIDTH / 4 - 6, FB_HEIGHT - 55, "CAM", 0xFFFFFF, 1);
    k_fill_circle(3 * FB_WIDTH / 4, FB_HEIGHT - 60, 20, 0x333333);
    k_draw_text(3 * FB_WIDTH / 4 - 8, FB_HEIGHT - 55, "LIGHT", 0xFFFFFF, 1);
}

/* --- Home screen (Springboard) --- */

void springboard_draw(void)
{
    k_clear(K_COLOR_SYSTEM_BG);

    /* Status bar */
    k_fill_rect(0, 0, FB_WIDTH, 40, K_COLOR_SYSTEM_BG);
    k_draw_text(20, 12, "KlikOS", 0x000000, 1);
    k_draw_text(FB_WIDTH - 120, 12, "12:00  100%", 0x000000, 1);
    k_draw_rect(0, 40, FB_WIDTH, 1, K_COLOR_SEPARATOR);

    /* App grid - 4 columns */
    u32 col_w = FB_WIDTH / 4;
    u32 row_h = 110;
    u32 start_y = 80;
    u32 icon_size = 60;

    for (int i = 0; i < 40; i++) {
        u32 col = i % 4;
        u32 row = i / 4;
        u32 x = col * col_w + (col_w - icon_size) / 2;
        u32 y = start_y + row * row_h + 10;

        draw_app_icon(x, y, icon_size, app_names[i], app_colors[i], app_glyphs[i]);
    }

    /* Dock */
    u32 dock_y = FB_HEIGHT - 110;
    k_fill_round_rect(20, dock_y, FB_WIDTH - 40, 70, 22, K_COLOR_SYSTEM_GROUPED);
    k_draw_rect(20, dock_y, FB_WIDTH - 40, 1, K_COLOR_SEPARATOR);

    const char *dock_names[4] = { "Phone", "Messages", "Browser", "App Store" };
    u32 dock_colors[4] = { 0x007AFF, 0x007AFF, 0x007AFF, 0x007AFF };
    const char *dock_glyphs[4] = { "PH", "MG", "BR", "AS" };
    u32 dock_icon = 50;
    u32 dock_spacing = (FB_WIDTH - 40 - 4 * dock_icon) / 5;

    for (int i = 0; i < 4; i++) {
        u32 x = 20 + dock_spacing + i * (dock_icon + dock_spacing);
        draw_app_icon(x, dock_y + 10, dock_icon, dock_names[i],
                      dock_colors[i], dock_glyphs[i]);
    }

    k_draw_text(FB_WIDTH / 2 - 10, FB_HEIGHT - 40, "1  2", 0x8E8E93, 1);
}

/* --- Control Center --- */

void control_center_draw(void)
{
    k_clear(0x000000);
    k_fill_rect(0, 0, FB_WIDTH, FB_HEIGHT, 0x1A1A1A);

    k_fill_rect(FB_WIDTH / 2 - 35, 15, 70, 6, 0x444444);
    k_draw_text(FB_WIDTH / 2 - 70, 50, "Control Center", 0xFFFFFF, 1);

    u32 items_w = 80, items_h = 80, gap = 30;
    u32 start_x = (FB_WIDTH - (4 * items_w + 3 * gap)) / 2;
    u32 start_y = 120;

    struct { u32 color; const char *glyph; const char *label; } items[4] = {
        { 0x007AFF, "AF", "Airplane" },
        { 0x8E8E93, "WI", "Wi-Fi" },
        { 0x8E8E93, "BT", "BT" },
        { 0x34C759, "CE", "Cell" }
    };

    for (int i = 0; i < 4; i++) {
        u32 x = start_x + i * (items_w + gap);
        k_fill_round_rect(x, start_y, items_w, items_h, 16, 0x333333);
        k_fill_circle(x + items_w / 2, start_y + 30, 18, items[i].color);
        k_draw_text(x + items_w / 2 - 6, start_y + 24, items[i].glyph, 0xFFFFFF, 2);
        k_draw_text(x + items_w / 2 - k_text_width(items[i].label, 1) / 2,
                    start_y + 60, items[i].label, 0xFFFFFF, 1);
    }

    k_draw_text(40, start_y + 130, "Brightness", 0xFFFFFF, 1);
    k_fill_rect(40, start_y + 150, 200, 6, 0x444444);
    k_fill_rect(40, start_y + 150, 150, 6, 0xFFFFFF);
    k_fill_circle(190, start_y + 153, 10, 0xFFFFFF);

    k_draw_text(40, start_y + 190, "Volume", 0xFFFFFF, 1);
    k_fill_rect(40, start_y + 210, 200, 6, 0x444444);
    k_fill_rect(40, start_y + 210, 100, 6, 0xFFFFFF);
    k_fill_circle(140, start_y + 213, 10, 0xFFFFFF);

    k_draw_text(FB_WIDTH / 2 - 40, FB_HEIGHT - 40, "Swipe up to close", 0x8E8E93, 1);
}

/* --- Settings --- */

static int toggle_wifi = 1;
static int toggle_bluetooth = 0;
static int toggle_dark = 0;

void settings_draw(void)
{
    k_clear(K_COLOR_SYSTEM_GROUPED);

    k_fill_rect(0, 0, FB_WIDTH, 90, K_COLOR_SYSTEM_GROUPED);
    k_draw_text(30, 30, "Settings", 0x000000, 2);

    u32 y = 110;
    u32 row_h = 70;

    k_fill_rect(30, y, FB_WIDTH - 60, row_h, 0xFFFFFF);
    k_draw_text(50, y + 12, "Airplane Mode", 0x000000, 1);
    k_fill_round_rect(FB_WIDTH - 80, y + 15, 50, 28, 14,
                      toggle_wifi ? 0x007AFF : 0xC6C6C8);
    k_fill_circle(toggle_wifi ? FB_WIDTH - 46 : FB_WIDTH - 66, y + 29, 10, 0xFFFFFF);
    y += row_h;

    k_fill_rect(30, y, FB_WIDTH - 60, row_h, 0xFFFFFF);
    k_draw_text(50, y + 12, "Wi-Fi", 0x000000, 1);
    k_draw_text(FB_WIDTH - 100, y + 12, toggle_wifi ? "On" : "Off", 0x3C3C43, 1);
    y += row_h;

    k_fill_rect(30, y, FB_WIDTH - 60, row_h, 0xFFFFFF);
    k_draw_text(50, y + 12, "Bluetooth", 0x000000, 1);
    k_draw_text(FB_WIDTH - 100, y + 12, toggle_bluetooth ? "On" : "Off", 0x3C3C43, 1);
    y += row_h;

    k_fill_rect(30, y, FB_WIDTH - 60, row_h, 0xFFFFFF);
    k_draw_text(50, y + 12, "Dark Mode", 0x000000, 1);
    k_fill_round_rect(FB_WIDTH - 80, y + 15, 50, 28, 14,
                      toggle_dark ? 0x007AFF : 0xC6C6C8);
    k_fill_circle(toggle_dark ? FB_WIDTH - 46 : FB_WIDTH - 66, y + 29, 10, 0xFFFFFF);
    y += row_h;

    k_fill_rect(30, y, FB_WIDTH - 60, row_h, 0xFFFFFF);
    k_draw_text(50, y + 12, "Display & Brightness", 0x000000, 1);
    k_draw_text(FB_WIDTH - 100, y + 12, ">", 0x8E8E93, 1);
    y += row_h;

    k_fill_rect(30, y, FB_WIDTH - 60, row_h, 0xFFFFFF);
    k_draw_text(50, y + 12, "Sounds & Haptics", 0x000000, 1);
    k_draw_text(FB_WIDTH - 100, y + 12, ">", 0x8E8E93, 1);
    y += row_h;

    k_fill_rect(30, y, FB_WIDTH - 60, row_h, 0xFFFFFF);
    k_draw_text(50, y + 12, "Wallpaper", 0x000000, 1);
    k_draw_text(FB_WIDTH - 100, y + 12, ">", 0x8E8E93, 1);
    y += row_h;

    k_fill_rect(30, y, FB_WIDTH - 60, row_h, 0xFFFFFF);
    k_draw_text(50, y + 12, "General", 0x000000, 1);
    k_draw_text(FB_WIDTH - 100, y + 12, ">", 0x8E8E93, 1);
    y += row_h;

    k_fill_rect(30, y, FB_WIDTH - 60, row_h, 0xFFFFFF);
    k_draw_text(50, y + 12, "About", 0x000000, 1);
    k_draw_text(FB_WIDTH - 100, y + 12, "KlikOS v0.1", 0x8E8E93, 1);

    k_draw_text(FB_WIDTH / 2 - 50, FB_HEIGHT - 40, "Swipe right to go back", 0x8E8E93, 1);
}

/* --- App window --- */

#define APP_WINDOW_W 300
#define APP_WINDOW_H 200

static void draw_app_window(const char *title, u32 accent, const char *content)
{
    k_clear(0xF2F2F7);

    k_fill_rect(0, 0, FB_WIDTH, 90, accent);
    k_fill_rect(0, 90, FB_WIDTH, 1, K_COLOR_SEPARATOR);
    k_draw_text(30, 30, title, 0xFFFFFF, 2);
    k_draw_text(20, 55, "< Back", 0xFFFFFF, 1);

    k_draw_text(30, 130, content, 0x000000, 1);
}

void app_launch(int index)
{
    if (index < 0 || index >= 40) return;

    ui_state = UI_STATE_APP;
    selected_app = index;

    kprintf("[app] Launching %s\n", app_names[index]);

    switch (index) {
    case 0:  draw_app_window("Settings", app_colors[index],
                             "Configure your device settings."); break;
    case 1:  draw_app_window("Files", app_colors[index],
                             "Browse your files and folders."); break;
    case 2:  draw_app_window("Terminal", app_colors[index],
                             "Command line interface."); break;
    case 3:  draw_app_window("Browser", app_colors[index],
                             "Browse the web."); break;
    case 4:  draw_app_window("App Store", app_colors[index],
                             "Download apps and games."); break;
    case 5:  draw_app_window("WhatsApp", app_colors[index],
                             "Message your friends."); break;
    case 6:  draw_app_window("Camera", app_colors[index],
                             "Take photos and videos."); break;
    case 7:  draw_app_window("Gallery", app_colors[index],
                             "View your photos."); break;
    case 8:  draw_app_window("Phone", app_colors[index],
                             "Make phone calls."); break;
    case 9:  draw_app_window("Messages", app_colors[index],
                             "Send text messages."); break;
    default: draw_app_window(app_names[index], app_colors[index],
                             app_names[index]); break;
    }

    k_fill_rect(FB_WIDTH / 2 - 60, FB_HEIGHT - 30, 120, 6, 0xC6C6C8);
    k_fill_round_rect(FB_WIDTH / 2 - 10, FB_HEIGHT - 24, 20, 4, 2, 0x8E8E93);
}

/* --- Input handling --- */

void springboard_tap(u32 x, u32 y)
{
    if (ui_state == UI_STATE_LOCK) {
        ui_state = UI_STATE_HOME;
        return;
    }

    if (ui_state == UI_STATE_HOME) {
        u32 dock_y = FB_HEIGHT - 110;
        if (y > dock_y && y < dock_y + 70) {
            u32 dock_spacing = (FB_WIDTH - 40 - 4 * 50) / 5;
            for (int i = 0; i < 4; i++) {
                u32 ix = 20 + dock_spacing + i * (50 + dock_spacing);
                if (x > ix && x < ix + 50 && y > dock_y + 10 && y < dock_y + 60) {
                    int idx = -1;
                    if (i == 0) idx = 8;
                    if (i == 1) idx = 9;
                    if (i == 2) idx = 3;
                    if (i == 3) idx = 4;
                    app_launch(idx);
                    return;
                }
            }
        }

        u32 col_w = FB_WIDTH / 4;
        u32 row_h = 110;
        u32 start_y = 80;

        if (y > start_y && y < FB_HEIGHT - 110) {
            u32 col = x / col_w;
            u32 row = (y - start_y) / row_h;
            int idx = row * 4 + col;
            if (idx >= 0 && idx < 40) {
                app_launch(idx);
                return;
            }
        }

        if (y < 40 && x > FB_WIDTH * 3 / 4) {
            ui_state = UI_STATE_CONTROL;
            return;
        }
    }

    if (ui_state == UI_STATE_APP) {
        if (y > FB_HEIGHT - 40) {
            ui_state = UI_STATE_HOME;
            return;
        }
    }

    if (ui_state == UI_STATE_CONTROL) {
        if (y > FB_HEIGHT - 60) {
            ui_state = UI_STATE_HOME;
        }
    }

    if (ui_state == UI_STATE_SETTINGS) {
        if (y > FB_HEIGHT - 60) {
            ui_state = UI_STATE_HOME;
        }
    }
}

void springboard_swipe(u32 from_x, u32 from_y, u32 to_x, u32 to_y)
{
    (void)from_x;
    if (from_y < 40 && to_y > 100 && from_x > FB_WIDTH * 2 / 3) {
        if (ui_state == UI_STATE_HOME) {
            ui_state = UI_STATE_CONTROL;
        }
    }
    if (from_y > FB_HEIGHT - 60 && to_y < FB_HEIGHT / 2) {
        if (ui_state == UI_STATE_LOCK) ui_state = UI_STATE_HOME;
        if (ui_state == UI_STATE_APP) ui_state = UI_STATE_HOME;
    }
}

/* --- Main draw dispatcher --- */

void ui_draw_frame(void)
{
    switch (ui_state) {
    case UI_STATE_LOCK:   lock_screen_draw(); break;
    case UI_STATE_HOME:   springboard_draw(); break;
    case UI_STATE_CONTROL: control_center_draw(); break;
    case UI_STATE_SETTINGS: settings_draw(); break;
    default: springboard_draw(); break;
    }
}
