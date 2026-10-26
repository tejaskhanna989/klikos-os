/* networking.c — KlikOS Networking abstraction.
 *
 * WiFi, cellular, Bluetooth networking stack stub.
 */

#include "networking.h"
#include <stdio.h>
#include <string.h>

static int net_initialized = 0;
static net_interface_t ifaces[NET_MAX_IFACES];
static int iface_count = 0;

void net_init(void) {
    memset(ifaces, 0, sizeof(ifaces));
    iface_count = 0;
    net_initialized = 1;
    printf("[networking] Initialized\n");
}

int net_scan_wifi(void) {
    if (!net_initialized) return -1;
    printf("[networking] Scanning WiFi networks...\n");
    return 0;
}

int net_connect_wifi(const char *ssid, const char *password) {
    (void)ssid; (void)password;
    if (!net_initialized) return -1;
    printf("[networking] Connecting to WiFi: %s\n", ssid);
    return 0;
}

int net_disconnect_wifi(void) {
    if (!net_initialized) return -1;
    printf("[networking] Disconnecting WiFi\n");
    return 0;
}

int net_get_interface(int idx, net_interface_t *iface) {
    if (!net_initialized || !iface || idx < 0 || idx >= iface_count) return -1;
    *iface = ifaces[idx];
    return 0;
}

int net_interface_count(void) {
    if (!net_initialized) return 0;
    return iface_count;
}

int net_send(const char *dst, const void *data, uint32_t size) {
    (void)dst; (void)data; (void)size;
    if (!net_initialized) return -1;
    printf("[networking] Send: %u bytes to %s\n", size, dst ? dst : "?");
    return (int)size;
}

int net_receive(char *src, void *buf, uint32_t size) {
    (void)src; (void)buf; (void)size;
    if (!net_initialized) return -1;
    return 0;
}

void net_cleanup(void) {
    net_initialized = 0;
    iface_count = 0;
    printf("[networking] Cleaned up\n");
}