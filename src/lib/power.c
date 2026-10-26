/* power.c — KlikOS Power management.
 *
 * Battery, charging, sleep/wake management stub.
 */

#include "power.h"
#include <stdio.h>

static power_battery_t battery = {
    .present = 1,
    .charging = 0,
    .level = 85,
    .time_remaining = 480,
    .temp = 25000,
};

static power_display_t display = {
    .screen_on = 1,
    .brightness = 70,
    .auto_sleep = 1,
    .sleep_timeout = 300,
};

void power_init(void) {
    printf("[power] Initialized\n");
}

power_battery_t *power_get_battery(void) {
    return &battery;
}

power_display_t *power_get_display(void) {
    return &display;
}

int power_set_brightness(uint8_t level) {
    if (level > 100) level = 100;
    display.brightness = level;
    printf("[power] Brightness: %u%%\n", level);
    return 0;
}

int power_set_sleep_timeout(uint32_t seconds) {
    display.sleep_timeout = seconds;
    printf("[power] Sleep timeout: %us\n", seconds);
    return 0;
}

int power_sleep(void) {
    display.screen_on = 0;
    printf("[power] Sleeping\n");
    return 0;
}

int power_wake(void) {
    display.screen_on = 1;
    printf("[power] Woke up\n");
    return 0;
}

int power_shutdown(void) {
    printf("[power] Shutting down\n");
    return 0;
}

int power_reboot(void) {
    printf("[power] Rebooting\n");
    return 0;
}

void power_cleanup(void) {
    printf("[power] Cleaned up\n");
}