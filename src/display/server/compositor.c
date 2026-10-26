/* display_server.c — KlikOS display server / compositor.
 *
 * Responsible for: framebuffer output, input (touch/keyboard), surface
 * management, compositing. Wayland-inspired but custom (not Wayland).
 */

#include <klikos/unistd.h>
#include <klikos/stdio.h>

int display_server_main(int argc, char **argv) {
    (void)argc; (void)argv;
    puts("[display] KlikOS display server starting");
    puts("[display] Framebuffer driver: stub");
    puts("[display] Compositor: stub");
    puts("[display] Waiting for events...");
    for (;;) {
        __asm__ volatile ("wfi");
    }
    return 0;
}
