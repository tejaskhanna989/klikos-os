/* app_store.c — KlikOS App Store backend.
 *
 * App signing, distribution, and installation stub.
 */

#include "app_store.h"
#include <stdio.h>
#include <string.h>

static int store_initialized = 0;
static app_store_entry_t installed_apps[64];
static int installed_count = 0;

void app_store_init(void) {
    store_initialized = 1;
    installed_count = 0;
    printf("[app_store] Initialized\n");
}

int app_store_search(const char *query) {
    (void)query;
    if (!store_initialized) return -1;
    printf("[app_store] Searching: %s\n", query ? query : "");
    return 0;
}

int app_store_download(const char *app_name) {
    (void)app_name;
    if (!store_initialized) return -1;
    printf("[app_store] Downloading: %s\n", app_name ? app_name : "");
    return 0;
}

int app_store_install(const char *app_name) {
    (void)app_name;
    if (!store_initialized) return -1;
    printf("[app_store] Installing: %s\n", app_name ? app_name : "");
    return 0;
}

int app_store_uninstall(const char *app_name) {
    (void)app_name;
    if (!store_initialized) return -1;
    printf("[app_store] Uninstalling: %s\n", app_name ? app_name : "");
    return 0;
}

int app_store_sign(const char *app_path, const char *key_path) {
    (void)app_path; (void)key_path;
    if (!store_initialized) return -1;
    printf("[app_store] Signing: %s with %s\n", app_path ? app_path : "", key_path ? key_path : "");
    return 0;
}

int app_store_verify_signature(const char *app_path, const char *sig_path) {
    (void)app_path; (void)sig_path;
    if (!store_initialized) return -1;
    printf("[app_store] Verifying signature: %s\n", app_path ? app_path : "");
    return 0;
}

int app_store_list_installed(app_store_entry_t *apps, int max) {
    if (!store_initialized || !apps) return -1;
    int count = installed_count > max ? max : installed_count;
    for (int i = 0; i < count; i++) {
        apps[i] = installed_apps[i];
    }
    return count;
}

void app_store_cleanup(void) {
    store_initialized = 0;
    installed_count = 0;
    printf("[app_store] Cleaned up\n");
}