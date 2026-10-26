/* input.h — KlikOS input subsystem.
 *
 * Handles keyboard, mouse, and touch input events.
 */

#ifndef KLIKOS_INPUT_H
#define KLIKOS_INPUT_H

#include <stdint.h>
#include <stddef.h>

/* Input event types */
#define KLIKOS_EVENT_KEYDOWN   1
#define KLIKOS_EVENT_KEYUP     2
#define KLIKOS_EVENT_MOUSEMOVE 3
#define KLIKOS_EVENT_MOUSEDOWN 4
#define KLIKOS_EVENT_MOUSEUP   5
#define KLIKOS_EVENT_TOUCHDOWN 6
#define KLIKOS_EVENT_TOUCHUP   7
#define KLIKOS_EVENT_TOUCHMOVE 8

typedef struct {
    int type;       /* KLIKOS_EVENT_* */
    int code;       /* key code or button code */
    int x, y;       /* cursor position */
    int pressure;   /* touch pressure (0-255) */
    int modifiers;  /* shift/ctrl/alt state */
    long timestamp; /* milliseconds */
} klikos_event_t;

/* Input device */
typedef struct {
    int fd;
    int type;       /* 0=keyboard, 1=mouse, 2=touchscreen */
    char name[64];
} klikos_input_dev_t;

/* Input subsystem */
int klikos_input_init(void);
int klikos_input_poll(klikos_event_t *ev, int max_events);
void klikos_input_cleanup(void);

/* Key codes */
#define KLIK_KEY_ESC      1
#define KLIK_KEY_1        2
#define KLIK_KEY_2        3
#define KLIK_KEY_3        4
#define KLIK_KEY_4        5
#define KLIK_KEY_5        6
#define KLIK_KEY_6        7
#define KLIK_KEY_7        8
#define KLIK_KEY_8        9
#define KLIK_KEY_9        10
#define KLIK_KEY_0        11
#define KLIK_KEY_MINUS    12
#define KLIK_KEY_EQUAL    13
#define KLIK_KEY_BACKSPACE 14
#define KLIK_KEY_TAB      15
#define KLIK_KEY_Q        16
#define KLIK_KEY_W        17
#define KLIK_KEY_E        18
#define KLIK_KEY_R        19
#define KLIK_KEY_T        20
#define KLIK_KEY_Y        21
#define KLIK_KEY_U        22
#define KLIK_KEY_I        23
#define KLIK_KEY_O        24
#define KLIK_KEY_P        25
#define KLIK_KEY_LEFTBRACE 26
#define KLIK_KEY_RIGHTBRACE 27
#define KLIK_KEY_ENTER    28
#define KLIK_KEY_LEFTCTRL 29
#define KLIK_KEY_A        30
#define KLIK_KEY_S        31
#define KLIK_KEY_D        32
#define KLIK_KEY_F        33
#define KLIK_KEY_G        34
#define KLIK_KEY_H        35
#define KLIK_KEY_J        36
#define KLIK_KEY_K        37
#define KLIK_KEY_L        38
#define KLIK_KEY_SEMICOLON 39
#define KLIK_KEY_APOSTROPHE 40
#define KLIK_KEY_GRAVE    41
#define KLIK_KEY_LEFTSHIFT 42
#define KLIK_KEY_BACKSLASH 43
#define KLIK_KEY_Z        44
#define KLIK_KEY_X        45
#define KLIK_KEY_C        46
#define KLIK_KEY_V        47
#define KLIK_KEY_B        48
#define KLIK_KEY_N        49
#define KLIK_KEY_M        50
#define KLIK_KEY_COMMA    51
#define KLIK_KEY_DOT      52
#define KLIK_KEY_SLASH    53
#define KLIK_KEY_RIGHTSHIFT 54
#define KLIK_KEY_KPASTERISK 55
#define KLIK_KEY_LEFTALT  56
#define KLIK_KEY_SPACE    57
#define KLIK_KEY_CAPSLOCK 58
#define KLIK_KEY_F1       59
#define KLIK_KEY_F2       60
#define KLIK_KEY_F3       61
#define KLIK_KEY_F4       62
#define KLIK_KEY_F5       63
#define KLIK_KEY_F6       64
#define KLIK_KEY_F7       65
#define KLIK_KEY_F8       66
#define KLIK_KEY_F9       67
#define KLIK_KEY_F10      68
#define KLIK_KEY_NUMLOCK  69
#define KLIK_KEY_SCROLLLOCK 70
#define KLIK_KEY_KP7      71
#define KLIK_KEY_KP8      72
#define KLIK_KEY_KP9      73
#define KLIK_KEY_KPMINUS  74
#define KLIK_KEY_KP4      75
#define KLIK_KEY_KP5      76
#define KLIK_KEY_KP6      77
#define KLIK_KEY_KPPLUS   78
#define KLIK_KEY_KP1      79
#define KLIK_KEY_KP2      80
#define KLIK_KEY_KP3      81
#define KLIK_KEY_KP0      82
#define KLIK_KEY_KPDOT    83
#define KLIK_KEY_F11      87
#define KLIK_KEY_F12      88

/* Mouse buttons */
#define KLIK_BTN_LEFT     1
#define KLIK_BTN_RIGHT    2
#define KLIK_BTN_MIDDLE   3

#endif /* KLIKOS_INPUT_H */