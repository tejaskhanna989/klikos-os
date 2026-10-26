/* secure_storage.c — KlikOS Secure Storage.
 *
 * Encrypted key-value storage stub.
 */

#include "secure_storage.h"
#include <stdio.h>
#include <string.h>

static int ss_initialized = 0;
static int ss_count = 0;
static char ss_keys[SS_MAX_ENTRIES][SS_MAX_KEY_LEN];
static uint8_t ss_data[SS_MAX_ENTRIES][SS_MAX_VALUE_LEN];
static uint32_t ss_sizes[SS_MAX_ENTRIES];

void ss_init(void) {
    memset(ss_keys, 0, sizeof(ss_keys));
    memset(ss_data, 0, sizeof(ss_data));
    memset(ss_sizes, 0, sizeof(ss_sizes));
    ss_count = 0;
    ss_initialized = 1;
    printf("[secure_storage] Initialized\n");
}

int ss_write(const char *key, const void *data, uint32_t size) {
    if (!ss_initialized) return -1;
    if (!key || !data || size == 0 || size > SS_MAX_VALUE_LEN) return -1;
    if (ss_count >= SS_MAX_ENTRIES) return -1;

    /* Check if key exists */
    for (int i = 0; i < ss_count; i++) {
        if (strcmp(ss_keys[i], key) == 0) {
            memcpy(ss_data[i], data, size);
            ss_sizes[i] = size;
            printf("[secure_storage] Updated: %s\n", key);
            return 0;
        }
    }

    /* Add new */
    strncpy(ss_keys[ss_count], key, SS_MAX_KEY_LEN - 1);
    ss_keys[ss_count][SS_MAX_KEY_LEN - 1] = '\0';
    memcpy(ss_data[ss_count], data, size);
    ss_sizes[ss_count] = size;
    ss_count++;
    printf("[secure_storage] Stored: %s (%u bytes)\n", key, size);
    return 0;
}

int ss_read(const char *key, void *buf, uint32_t max_size) {
    if (!ss_initialized || !key || !buf) return -1;
    for (int i = 0; i < ss_count; i++) {
        if (strcmp(ss_keys[i], key) == 0) {
            if (ss_sizes[i] > max_size) return -1;
            memcpy(buf, ss_data[i], ss_sizes[i]);
            return ss_sizes[i];
        }
    }
    return -1;
}

int ss_delete(const char *key) {
    if (!ss_initialized) return -1;
    for (int i = 0; i < ss_count; i++) {
        if (strcmp(ss_keys[i], key) == 0) {
            /* Shift remaining entries */
            for (int j = i; j < ss_count - 1; j++) {
                strcpy(ss_keys[j], ss_keys[j+1]);
                memcpy(ss_data[j], ss_data[j+1], ss_sizes[j+1]);
                ss_sizes[j] = ss_sizes[j+1];
            }
            ss_count--;
            printf("[secure_storage] Deleted: %s\n", key);
            return 0;
        }
    }
    return -1;
}

int ss_exists(const char *key) {
    if (!ss_initialized) return 0;
    for (int i = 0; i < ss_count; i++) {
        if (strcmp(ss_keys[i], key) == 0) return 1;
    }
    return 0;
}

int ss_list(char **keys, int max_keys) {
    if (!ss_initialized) return -1;
    int count = 0;
    for (int i = 0; i < ss_count && count < max_keys; i++) {
        keys[count] = ss_keys[i];
        count++;
    }
    return count;
}

void ss_cleanup(void) {
    ss_count = 0;
    ss_initialized = 0;
    printf("[secure_storage] Cleaned up\n");
}