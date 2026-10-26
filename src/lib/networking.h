/* networking.h — KlikOS Networking abstraction.
 *
 * WiFi, cellular, Bluetooth networking stack.
 */

#ifndef KLIKOS_NETWORKING_H
#define KLIKOS_NETWORKING_H

#include <stdint.h>
#include <stddef.h>

#define NET_MAX_IFACES 8
#define NET_MAX_SSID 32

typedef struct {
    char name[32];
    char type[16];       /* wifi, cellular, bluetooth, ethernet */
    int connected;
    uint32_t ip_addr;
    char ssid[NET_MAX_SSID];
    int signal_strength; /* 0-100 */
} net_interface_t;

/* Networking functions */
void net_init(void);
int net_scan_wifi(void);
int net_connect_wifi(const char *ssid, const char *password);
int net_disconnect_wifi(void);
int net_get_interface(int idx, net_interface_t *iface);
int net_interface_count(void);
int net_send(const char *dst, const void *data, uint32_t size);
int net_receive(char *src, void *buf, uint32_t size);

void net_cleanup(void);

#endif /* KLIKOS_NETWORKING_H */