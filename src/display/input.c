/* input.c — KlikOS input subsystem.
 *
 * Handles keyboard, mouse, and touch input from Linux input devices (/dev/input/*)
 * and translates them into KlikOS events.
 */

#define _GNU_SOURCE
#include "input.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <linux/input.h>
#include <time.h>
#include <errno.h>

#define MAX_DEVICES 16
#define MAX_EVENTS 64

static klikos_input_dev_t devices[MAX_DEVICES];
static int device_count = 0;

/* Scan /dev/input for input devices */
static int scan_input_devices(void) {
    DIR *d = opendir("/dev/input");
    if (!d) {
        fprintf(stderr, "[input] Cannot open /dev/input\n");
        return 0;
    }

    struct dirent *entry;
    while ((entry = readdir(d)) != NULL) {
        if (strncmp(entry->d_name, "event", 5) != 0)
            continue;

        char path[256];
        snprintf(path, sizeof(path), "/dev/input/%s", entry->d_name);

        int fd = open(path, O_RDONLY | O_NONBLOCK);
        if (fd < 0) continue;

        /* Identify device type */
        char name[256] = "unknown";
        ioctl(fd, EVIOCGNAME(sizeof(name)), name);

        int type = -1;
        if (strcasestr(name, "keyboard") || strcasestr(name, "kbd") ||
            strcasestr(name, "at")) {
            type = 0; /* keyboard */
        } else if (strcasestr(name, "mouse")) {
            type = 1; /* mouse */
        } else if (strcasestr(name, "touch") || strcasestr(name, "touchscreen")) {
            type = 2; /* touchscreen */
        } else {
            /* Check capabilities */
            unsigned long evbit[EV_MAX/8 + 1] = {0};
            ioctl(fd, EVIOCGBIT(0, sizeof(evbit)), evbit);
            if (evbit[EV_KEY/8] & (1 << (EV_KEY % 8))) {
                type = 0; /* keyboard */
            } else if (evbit[EV_REL/8] & (1 << (EV_REL % 8))) {
                type = 1; /* mouse */
            } else if (evbit[EV_ABS/8] & (1 << (EV_ABS % 8))) {
                type = 2; /* touchscreen */
            }
        }

        if (device_count < MAX_DEVICES) {
            devices[device_count].fd = fd;
            devices[device_count].type = type;
            strncpy(devices[device_count].name, name, sizeof(devices[device_count].name) - 1);
            device_count++;
            printf("[input] Found device: %s (type=%d)\n", name, type);
        }
    }

    closedir(d);
    return device_count;
}

int klikos_input_init(void) {
    device_count = 0;
    int count = scan_input_devices();
    printf("[input] Initialized %d input device(s)\n", count);
    return count;
}

int klikos_input_poll(klikos_event_t *ev, int max_events) {
    if (!ev || max_events <= 0) return 0;

    int count = 0;
    struct input_event ev_raw[MAX_EVENTS];

    for (int d = 0; d < device_count && count < max_events; d++) {
        int fd = devices[d].fd;
        if (fd < 0) continue;

        int n = read(fd, ev_raw, sizeof(ev_raw));
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) continue;
            continue;
        }

        int n_events = n / sizeof(struct input_event);
        for (int i = 0; i < n_events && count < max_events; i++) {
            struct input_event *ev_in = &ev_raw[i];

            ev->timestamp = ev_in->time.tv_sec * 1000 + ev_in->time.tv_usec / 1000;

            switch (ev_in->type) {
            case EV_KEY:
                if (ev_in->value == 0 || ev_in->value == 1) {
                    ev->type = (ev_in->value == 1) ? KLIKOS_EVENT_KEYDOWN : KLIKOS_EVENT_KEYUP;
                    ev->code = ev_in->code;
                    ev->x = 0;
                    ev->y = 0;
                    ev->pressure = 0;
                    ev->modifiers = 0;
                    count++;
                    ev++;
                }
                break;

            case EV_REL:
                if (ev_in->code == REL_X || ev_in->code == REL_Y) {
                    static int mouse_x = 0, mouse_y = 0;
                    if (ev_in->code == REL_X) mouse_x += ev_in->value;
                    if (ev_in->code == REL_Y) mouse_y += ev_in->value;
                    ev->type = KLIKOS_EVENT_MOUSEMOVE;
                    ev->code = 0;
                    ev->x = mouse_x;
                    ev->y = mouse_y;
                    ev->pressure = 0;
                    ev->modifiers = 0;
                    count++;
                    ev++;
                }
                break;

            case EV_ABS:
                if (ev_in->code == ABS_X) {
                    ev->type = KLIKOS_EVENT_MOUSEMOVE;
                    ev->code = 0;
                    ev->x = ev_in->value;
                    ev->pressure = 0;
                    ev->modifiers = 0;
                    count++;
                    ev++;
                } else if (ev_in->code == ABS_Y) {
                    if (count > 0) ev[-1].y = ev_in->value;
                } else if (ev_in->code == ABS_PRESSURE) {
                    if (count > 0) ev[-1].pressure = ev_in->value;
                } else if (ev_in->code == BTN_TOUCH || ev_in->code == BTN_TOOL_FINGER) {
                    if (ev_in->value == 1) {
                        ev->type = KLIKOS_EVENT_TOUCHDOWN;
                        ev->code = 0;
                        ev->x = 0;
                        ev->y = 0;
                        ev->pressure = 255;
                        ev->modifiers = 0;
                        count++;
                        ev++;
                    } else if (ev_in->value == 0) {
                        ev->type = KLIKOS_EVENT_TOUCHUP;
                        ev->code = 0;
                        ev->x = 0;
                        ev->y = 0;
                        ev->pressure = 0;
                        ev->modifiers = 0;
                        count++;
                        ev++;
                    }
                }
                break;
            }
        }
    }

    return count;
}

void klikos_input_cleanup(void) {
    for (int i = 0; i < device_count; i++) {
        if (devices[i].fd >= 0) close(devices[i].fd);
    }
    device_count = 0;
}