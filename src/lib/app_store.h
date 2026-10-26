/* app_store.h — KlikOS App Store backend.
 *
 * App signing, distribution, and installation.
 */

#ifndef KLIKOS_APP_STORE_H
#define KLIKOS_APP_STORE_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    char name[64];
    char version[32];
    char developer[64];
    uint32_t size;
    uint8_t verified;
    uint8_t installed;
} app_store_entry_t;

typedef struct {
    char name[64];
    uint8_t valid;
} app_signature_t;

/* App store functions */
void app_store_init(void);
int app_store_search(const char *query);
int app_store_download(const char *app_name);
int app_store_install(const char *app_name);
int app_store_uninstall(const char *app_name);
int app_store_sign(const char *app_path, const char *key_path);
int app_store_verify_signature(const char *app_path, const char *sig_path);
int app_store_list_installed(app_store_entry_t *apps, int max);

void app_store_cleanup(void);

#endif /* KLIKOS_APP_STORE_H */