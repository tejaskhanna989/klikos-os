/* main.c - KlikOS userspace entry point (PID 1 after kernel boot).
 *
 * Initializes: display server, toolkit, shell (with window manager + input),
 * app framework, filesystem, networking, power, secure storage, update,
 * app store, then runs the main loop handling input events and redrawing.
 */

#define _DEFAULT_SOURCE
#include "display.h"
#include "toolkit.h"
#include "shell.h"
#include "framework.h"
#include "filesystem.h"
#include "networking.h"
#include "power.h"
#include "secure_storage.h"
#include "update.h"
#include "app_store.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/mount.h>

static volatile int running = 1;

static void handle_signal(int sig) {
    (void)sig;
    running = 0;
}

int main(int argc, char **argv) {
    (void)argc; (void)argv;

    printf("[klikos] Userspace starting...\n");

    /* Mount devtmpfs so /dev/fb0 and other device nodes appear */
    printf("[klikos] Mounting devtmpfs...\n");
    if (mount("devtmpfs", "/dev", "devtmpfs", 0, "") != 0) {
        printf("[klikos] Warning: could not mount devtmpfs, /dev/fb0 may not exist\n");
    }

    printf("[klikos] Initializing display server...\n");

    /* Initialize display - tries /dev/fb0 first, then PCI BAR0 via /dev/mem */
    k_display_t disp;
    if (k_display_init(&disp, NULL) != 0) {
        printf("[klikos] Error: display init failed\n");
        return 1;
    }

    printf("[klikos] Display: %ux%u stride=%u bpp=%u\n",
           disp.width, disp.height, disp.stride, disp.bpp);

    /* Initialize shell (includes window manager + input) */
    k_shell_t shell;
    k_shell_init(&shell, &disp);

    /* Initialize input subsystem */
    printf("[klikos] Initializing input subsystem...\n");
    int input_devs = klikos_input_init();
    printf("[klikos] Input: %d device(s) found\n", input_devs);

    /* Initialize app framework */
    printf("[klikos] Initializing app framework...\n");
    framework_init();

    /* Initialize filesystem */
    printf("[klikos] Initializing filesystem...\n");
    fs_init();

    /* Initialize networking */
    printf("[klikos] Initializing networking...\n");
    net_init();

    /* Initialize power management */
    printf("[klikos] Initializing power management...\n");
    power_init();

    /* Initialize secure storage */
    printf("[klikos] Initializing secure storage...\n");
    ss_init();

    /* Initialize update system */
    printf("[klikos] Initializing update system...\n");
    update_init();

    /* Initialize app store */
    printf("[klikos] Initializing app store...\n");
    app_store_init();

    /* Register built-in apps with framework */
    const char *app_names[40] = {
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
    for (int i = 0; i < 40; i++) {
        framework_register_app(app_names[i], app_names[i],
                              k_color_from_hex(0xFF007AFF));
    }

    /* Initial draw */
    k_shell_draw(&shell);
    k_display_flip(&disp);

    printf("[klikos] Entering main loop...\n");

    /* Signal handling */
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    /* Main loop - input + draw at ~60fps */
    while (running) {
        /* Process input events */
        klikos_event_t ev;
        while (klikos_input_poll(&ev, 16) > 0) {
            k_shell_handle_event(&shell, &ev);
        }

        /* Redraw at ~60fps */
        usleep(16667);
        k_shell_draw(&shell);
        k_display_flip(&disp);
    }

    /* Cleanup */
    k_shell_cleanup(&shell);
    klikos_input_cleanup();
    k_display_cleanup(&disp);
    framework_cleanup();
    fs_cleanup();
    net_cleanup();
    power_cleanup();
    ss_cleanup();
    update_cleanup();
    app_store_cleanup();

    printf("[klikos] Userspace shutting down...\n");
    return 0;
}