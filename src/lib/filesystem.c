/* filesystem.c — KlikOS Filesystem abstraction.
 *
 * Minimal filesystem abstraction layer.
 */

#include "filesystem.h"
#include <stdio.h>
#include <string.h>

static int fs_mounted = 0;

void fs_init(void) {
    fs_mounted = 1;
    printf("[filesystem] Initialized\n");
}

int fs_mount(const char *device, const char *mountpoint) {
    (void)device; (void)mountpoint;
    fs_mounted = 1;
    printf("[filesystem] Mounted %s at %s\n", device, mountpoint);
    return 0;
}

int fs_unmount(const char *mountpoint) {
    (void)mountpoint;
    fs_mounted = 0;
    printf("[filesystem] Unmounted %s\n", mountpoint);
    return 0;
}

int fs_create_file(const char *path) {
    (void)path;
    if (!fs_mounted) return -1;
    printf("[filesystem] Create file: %s\n", path);
    return 0;
}

int fs_delete_file(const char *path) {
    (void)path;
    if (!fs_mounted) return -1;
    printf("[filesystem] Delete file: %s\n", path);
    return 0;
}

int fs_write_file(const char *path, const void *data, uint32_t size) {
    (void)path; (void)data; (void)size;
    if (!fs_mounted) return -1;
    printf("[filesystem] Write file: %s (%u bytes)\n", path, size);
    return (int)size;
}

int fs_read_file(const char *path, void *buf, uint32_t size) {
    (void)path; (void)buf; (void)size;
    if (!fs_mounted) return -1;
    printf("[filesystem] Read file: %s (%u bytes)\n", path, size);
    return 0;
}

int fs_file_exists(const char *path) {
    (void)path;
    if (!fs_mounted) return 0;
    return 0;
}

int fs_list_dir(const char *path, fs_file_info_t *entries, int max_entries) {
    (void)path; (void)entries; (void)max_entries;
    if (!fs_mounted) return -1;
    return 0;
}

uint32_t fs_get_free_space(const char *path) {
    (void)path;
    if (!fs_mounted) return 0;
    return 0;
}

void fs_cleanup(void) {
    fs_mounted = 0;
    printf("[filesystem] Cleaned up\n");
}