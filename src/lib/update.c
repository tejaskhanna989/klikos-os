/* update.c — KlikOS OTA Update system.
 *
 * Over-the-air update stub.
 */

#include "update.h"
#include <stdio.h>
#include <string.h>

static int update_initialized = 0;
static char current_version[32] = "0.1.0";

void update_init(void) {
    update_initialized = 1;
    printf("[update] Initialized (version %s)\n", current_version);
}

int update_check(void) {
    if (!update_initialized) return -1;
    printf("[update] Checking for updates...\n");
    return 0;
}

int update_download(const char *url) {
    (void)url;
    if (!update_initialized) return -1;
    printf("[update] Downloading from: %s\n", url);
    return 0;
}

int update_verify(const char *path) {
    (void)path;
    if (!update_initialized) return -1;
    printf("[update] Verifying: %s\n", path);
    return 0;
}

int update_install(const char *path) {
    (void)path;
    if (!update_initialized) return -1;
    printf("[update] Installing: %s\n", path);
    return 0;
}

int update_rollback(void) {
    if (!update_initialized) return -1;
    printf("[update] Rolling back\n");
    return 0;
}

int update_get_version(char *buf, uint32_t size) {
    if (!buf || size == 0) return -1;
    strncpy(buf, current_version, size - 1);
    buf[size - 1] = '\0';
    return 0;
}

void update_cleanup(void) {
    update_initialized = 0;
    printf("[update] Cleaned up\n");
}