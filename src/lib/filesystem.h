/* filesystem.h — KlikOS Filesystem abstraction.
 *
 * Provides file storage abstraction for the OS.
 */

#ifndef KLIKOS_FILESYSTEM_H
#define KLIKOS_FILESYSTEM_H

#include <stdint.h>
#include <stddef.h>

#define FS_MAX_PATH 256
#define FS_MAX_FILES 1024

typedef struct {
    char name[FS_MAX_PATH];
    uint32_t size;
    uint32_t created;
    uint32_t modified;
    uint8_t is_dir;
} fs_file_info_t;

/* Filesystem functions */
void fs_init(void);
int fs_mount(const char *device, const char *mountpoint);
int fs_unmount(const char *mountpoint);
int fs_create_file(const char *path);
int fs_delete_file(const char *path);
int fs_write_file(const char *path, const void *data, uint32_t size);
int fs_read_file(const char *path, void *buf, uint32_t size);
int fs_file_exists(const char *path);
int fs_list_dir(const char *path, fs_file_info_t *entries, int max_entries);
uint32_t fs_get_free_space(const char *path);

void fs_cleanup(void);

#endif /* KLIKOS_FILESYSTEM_H */