/* framework.h — KlikOS App Framework.
 *
 * App lifecycle management, IPC, and app registry.
 */

#ifndef KLIKOS_FRAMEWORK_H
#define KLIKOS_FRAMEWORK_H

#include <stdint.h>
#include <stddef.h>
#include "display.h"

/* App states */
#define APP_STATE_STOPPED   0
#define APP_STATE_RUNNING   1
#define APP_STATE_PAUSED    2
#define APP_STATE_DESTROYED 3

/* App message types for IPC */
#define MSG_APP_LAUNCH      1
#define MSG_APP_CLOSE       2
#define MSG_APP_DATA        3
#define Msg_APP_QUIT        4

typedef struct {
    int id;
    char name[64];
    char package[128];
    int state;
    int pid;
    k_color_t color;
    int window_id;
    void *user_data;
} app_info_t;

/* IPC message */
typedef struct {
    int msg_type;
    int src_app_id;
    int dst_app_id;
    uint32_t data_len;
    void *data;
} app_msg_t;

/* Framework functions */
void framework_init(void);
int framework_register_app(const char *name, const char *package, k_color_t color);
int framework_launch_app(const char *name);
int framework_close_app(int app_id);
int framework_send_message(app_msg_t *msg);
app_info_t *framework_get_app(int app_id);
int framework_app_count(void);
void framework_cleanup(void);

/* App callback types */
typedef void (*app_on_create)(int app_id);
typedef void (*app_on_destroy)(int app_id);
typedef void (*app_on_pause)(int app_id);
typedef void (*app_on_resume)(int app_id);
typedef void (*app_on_msg)(int app_id, app_msg_t *msg);

typedef struct {
    app_on_create on_create;
    app_on_destroy on_destroy;
    app_on_pause on_pause;
    app_on_resume on_resume;
    app_on_msg on_msg;
} app_callbacks_t;

int framework_register_callbacks(int app_id, app_callbacks_t *cb);

#endif /* KLIKOS_FRAMEWORK_H */