/* framework.c — KlikOS App Framework.
 *
 * App lifecycle management, registration, and IPC message passing.
 */

#include "framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_APPS 64
#define MAX_MSG_QUEUE 128

static app_info_t apps[MAX_APPS];
static int app_count = 0;
static app_msg_t msg_queue[MAX_MSG_QUEUE];
static int msg_head = 0;
static int msg_tail = 0;
static int msg_count = 0;

static app_callbacks_t app_callbacks[MAX_APPS];

void framework_init(void) {
    memset(apps, 0, sizeof(apps));
    memset(app_callbacks, 0, sizeof(app_callbacks));
    app_count = 0;
    msg_head = 0;
    msg_tail = 0;
    msg_count = 0;
    printf("[framework] App framework initialized\n");
}

int framework_register_app(const char *name, const char *package, k_color_t color) {
    if (app_count >= MAX_APPS) {
        fprintf(stderr, "[framework] Max apps reached\n");
        return -1;
    }

    app_info_t *app = &apps[app_count];
    app->id = app_count + 1;
    strncpy(app->name, name, sizeof(app->name) - 1);
    app->name[sizeof(app->name) - 1] = '\0';
    strncpy(app->package, package, sizeof(app->package) - 1);
    app->package[sizeof(app->package) - 1] = '\0';
    app->state = APP_STATE_STOPPED;
    app->pid = 0;
    app->color = color;
    app->window_id = -1;
    app->user_data = NULL;

    app_count++;
    printf("[framework] Registered app: %s (id=%d)\n", name, app->id);
    return app->id;
}

int framework_launch_app(const char *name) {
    for (int i = 0; i < app_count; i++) {
        if (strcmp(apps[i].name, name) == 0) {
            apps[i].state = APP_STATE_RUNNING;
            apps[i].pid = i + 1; /* mock PID */
            printf("[framework] Launched app: %s (id=%d)\n", name, apps[i].id);

            /* Call on_create callback */
            if (app_callbacks[i].on_create) {
                app_callbacks[i].on_create(apps[i].id);
            }

            return apps[i].id;
        }
    }
    fprintf(stderr, "[framework] App not found: %s\n", name);
    return -1;
}

int framework_close_app(int app_id) {
    for (int i = 0; i < app_count; i++) {
        if (apps[i].id == app_id) {
            apps[i].state = APP_STATE_STOPPED;

            /* Call on_destroy callback */
            if (app_callbacks[i].on_destroy) {
                app_callbacks[i].on_destroy(apps[i].id);
            }

            printf("[framework] Closed app: %s (id=%d)\n", apps[i].name, app_id);
            return 0;
        }
    }
    return -1;
}

int framework_send_message(app_msg_t *msg) {
    if (!msg) return -1;
    if (msg_count >= MAX_MSG_QUEUE) return -1;

    msg_queue[msg_tail] = *msg;
    msg_tail = (msg_tail + 1) % MAX_MSG_QUEUE;
    msg_count++;

    /* Deliver to destination app */
    for (int i = 0; i < app_count; i++) {
        if (apps[i].id == msg->dst_app_id && app_callbacks[i].on_msg) {
            app_callbacks[i].on_msg(apps[i].id, msg);
        }
    }

    return 0;
}

app_info_t *framework_get_app(int app_id) {
    for (int i = 0; i < app_count; i++) {
        if (apps[i].id == app_id) {
            return &apps[i];
        }
    }
    return NULL;
}

int framework_app_count(void) {
    return app_count;
}

int framework_register_callbacks(int app_id, app_callbacks_t *cb) {
    for (int i = 0; i < app_count; i++) {
        if (apps[i].id == app_id) {
            app_callbacks[i] = *cb;
            return 0;
        }
    }
    return -1;
}

void framework_cleanup(void) {
    app_count = 0;
    msg_count = 0;
    msg_head = 0;
    msg_tail = 0;
    memset(app_callbacks, 0, sizeof(app_callbacks));
    printf("[framework] Framework cleaned up\n");
}