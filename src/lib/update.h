/* update.h — KlikOS OTA Update system.
 */

#ifndef KLIKOS_UPDATE_H
#define KLIKOS_UPDATE_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    char version[32];
    uint32_t size;
    uint32_t checksum;
    uint8_t verified;
} update_package_t;

/* Update functions */
void update_init(void);
int update_check(void);
int update_download(const char *url);
int update_verify(const char *path);
int update_install(const char *path);
int update_rollback(void);
int update_get_version(char *buf, uint32_t size);

void update_cleanup(void);

#endif /* KLIKOS_UPDATE_H */