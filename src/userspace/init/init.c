/* init.c — KlikOS init (PID 1).
 *
 * First userspace process. Mounts filesystems, starts core daemons,
 * launches the display server and mobile shell, then reaps orphans.
 */

#include <klikos/unistd.h>
#include <klikos/stdio.h>
#include <klikos/string.h>

void mount_rootfs(void);
void start_daemons(void);
void start_display_server(void);
void start_shell(void);
void reap_loop(void);
int fork_stub(void);

int main(void) {
    puts("[init] KlikOS init starting (PID 1)");

    /* Mount root and filesystems */
    mount_rootfs();

    /* Start core system daemons */
    start_daemons();

    /* Start the display server / compositor */
    start_display_server();

    /* Start the mobile shell (launcher) */
    start_shell();

    /* Reap orphaned processes forever */
    reap_loop();

    /* Should never reach here */
    return 0;
}

void mount_rootfs(void) {
    puts("[init] Mounting rootfs...");
    /* Stub — real mount syscalls wired later */
}

void start_daemons(void) {
    puts("[init] Starting daemons...");
    /* ueventd, logd, netd, etc. — stubs */
}

void start_display_server(void) {
    puts("[init] Starting display server...");
    int pid = fork_stub();
    if (pid == 0) {
        /* Display server stub (real one built separately as /bin/display-server) */
        puts("[display] stub running inline");
        for (;;) { __asm__ volatile ("wfi"); }
    }
}

void start_shell(void) {
    puts("[init] Starting shell...");
    int pid = fork_stub();
    if (pid == 0) {
        /* Shell stub (real one built separately as /bin/sh) */
        puts("[shell] stub running inline");
        for (;;) { __asm__ volatile ("wfi"); }
    }
}

int fork_stub(void) {
    /* No process creation yet — return 0 so we run inline */
    return 0;
}

void reap_loop(void) {
    for (;;) {
        __asm__ volatile ("wfi");
    }
}
