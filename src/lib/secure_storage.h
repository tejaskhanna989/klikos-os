/* secure_storage.h — KlikOS Secure Storage.
 *
 * Encrypted key-value storage for sensitive data.
 */

#ifndef KLIKOS_SECURE_STORAGE_H
#define KLIKOS_SECURE_STORAGE_H

#include <stdint.h>
#include <stddef.h>

#define SS_MAX_KEY_LEN 64
#define SS_MAX_VALUE_LEN 4096
#define SS_MAX_ENTRIES 256

/* Secure storage functions */
void ss_init(void);
int ss_write(const char *key, const void *data, uint32_t size);
int ss_read(const char *key, void *buf, uint32_t max_size);
int ss_delete(const char *key);
int ss_exists(const char *key);
int ss_list(char **keys, int max_keys);

void ss_cleanup(void);

#endif /* KLIKOS_SECURE_STORAGE_H */