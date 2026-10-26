/* apps/Weather.c - KlikOS Weather app */

#define _DEFAULT_SOURCE
#include "display.h"
#include "toolkit.h"
#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

static volatile int running = 1;

static void handle_signal(int sig) {
    (void)sig;
    running = 0;
}

int main(int argc, char **argv) {
    (void)argc; (void)argv;

    const char *app_name = "Weather";
    if (argc > 1) app_name = argv[1];

    printf("[%s] Starting...\n", app_name);

    k_display_t disp;
    if (k_display_init(&disp, "/dev/dri/card0") != 0) {
        k_display_init(&disp, NULL);
    }

    k_draw_fill_rect(&disp, (k_rect_t){0, 0, disp.width, disp.height},
                     k_color(242, 242, 247, 255));

    k_draw_fill_rect(&disp, (k_rect_t){0, 0, disp.width, 90},
                     k_color_from_hex(0xFF007AFF));
    k_draw_text(&disp, 30, 30, app_name, k_color(255,255,255,255), 2);
    k_draw_text(&disp, 20, 55, "< Back", k_color(255,255,255,255), 1);

    char content[256];
    snprintf(content, sizeof(content), "%s - Application Content", app_name);
    k_draw_text(&disp, 30, 130, content, k_color(0,0,0,255), 1);

    k_draw_fill_rect(&disp, (k_rect_t){disp.width/2 - 60, disp.height - 30,
                                       120, 6}, k_color(198,198,200,255));
    k_draw_fill_round_rect(&disp, (k_rect_t){disp.width/2 - 10, disp.height - 24,
                                             20, 4}, 2,
                           k_color(142,142,147,255));

    k_display_flip(&disp);

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    while (running) {
        usleep(100000);
    }

    k_display_cleanup(&disp);
    printf("[%s] Shutting down...\n", app_name);
    return 0;
}
