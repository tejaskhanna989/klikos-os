/* power.h — KlikOS Power management.
 *
 * Battery, charging, sleep/wake management.
 */

#ifndef KLIKOS_POWER_H
#define KLIKOS_POWER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    int present;
    int charging;
    uint8_t level;      /* 0-100 */
    uint32_t time_remaining; /* minutes */
    uint32_t temp;      /* millidegrees C */
} power_battery_t;

typedef struct {
    bool screen_on;
    uint32_t brightness; /* 0-100 */
    bool auto_sleep;
    uint32_t sleep_timeout; /* seconds */
} power_display_t;

/* Power functions */
void power_init(void);
power_battery_t *power_get_battery(void);
power_display_t *power_get_display(void);
int power_set_brightness(uint8_t level);
int power_set_sleep_timeout(uint32_t seconds);
int power_sleep(void);
int power_wake(void);
int power_shutdown(void);
int power_reboot(void);

void power_cleanup(void);

#endif /* KLIKOS_POWER_H */