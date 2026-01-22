#pragma once

#include <stdint.h>

// HID Report Descriptor macros
#define USAGE_PAGE(n)       0x05, n
#define USAGE(n)            0x09, n
#define COLLECTION(n)       0xA1, n
#define END_COLLECTION(n)   0xC0
#define REPORT_ID(n)        0x85, n
#define USAGE_MINIMUM(n)    0x19, n
#define USAGE_MAXIMUM(n)    0x29, n
#define LOGICAL_MINIMUM(n)  0x15, n
#define LOGICAL_MAXIMUM(n)  0x25, n
#define REPORT_SIZE(n)      0x75, n
#define REPORT_COUNT(n)     0x95, n
#define HIDINPUT(n)         0x81, n
#define HIDOUTPUT(n)        0x91, n

// HID Appearance
#define HID_KEYBOARD 0x03C1

// Key report structure (8 bytes: modifiers + reserved + 6 keys)
typedef struct {
  uint8_t modifiers;
  uint8_t reserved;
  uint8_t keys[6];
} KeyReport;

// Media key report (2 bytes)
typedef uint8_t MediaKeyReport[2];

// ASCII map for keyboard (from Arduino Keyboard library)
extern const uint8_t _asciimap[128] PROGMEM;

// Modifier keys
#define KEY_LEFT_CTRL   0x80
#define KEY_LEFT_SHIFT  0x81
#define KEY_LEFT_ALT    0x82
#define KEY_LEFT_GUI    0x83
#define KEY_RIGHT_CTRL  0x84
#define KEY_RIGHT_SHIFT 0x85
#define KEY_RIGHT_ALT   0x86
#define KEY_RIGHT_GUI   0x87

// Special keys
#define KEY_UP_ARROW    0xDA
#define KEY_DOWN_ARROW  0xD9
#define KEY_LEFT_ARROW  0xD8
#define KEY_RIGHT_ARROW 0xD7
#define KEY_BACKSPACE   0xB2
#define KEY_TAB         0xB3
#define KEY_RETURN      0xB0
#define KEY_ESC         0xB1
#define KEY_INSERT      0xD1
#define KEY_DELETE      0xD4
#define KEY_PAGE_UP     0xD3
#define KEY_PAGE_DOWN   0xD6
#define KEY_HOME        0xD2
#define KEY_END         0xD5
#define KEY_CAPS_LOCK   0xC1
#define KEY_PRTSC       0xCE

// Function keys
#define KEY_F1          0xC2
#define KEY_F2          0xC3
#define KEY_F3          0xC4
#define KEY_F4          0xC5
#define KEY_F5          0xC6
#define KEY_F6          0xC7
#define KEY_F7          0xC8
#define KEY_F8          0xC9
#define KEY_F9          0xCA
#define KEY_F10         0xCB
#define KEY_F11         0xCC
#define KEY_F12         0xCD
#define KEY_F13         0xF0
#define KEY_F14         0xF1
#define KEY_F15         0xF2
#define KEY_F16         0xF3
#define KEY_F17         0xF4
#define KEY_F18         0xF5
#define KEY_F19         0xF6
#define KEY_F20         0xF7
#define KEY_F21         0xF8
#define KEY_F22         0xF9
#define KEY_F23         0xFA
#define KEY_F24         0xFB

// Numpad keys
#define KEY_NUM_0       0xEA
#define KEY_NUM_1       0xE1
#define KEY_NUM_2       0xE2
#define KEY_NUM_3       0xE3
#define KEY_NUM_4       0xE4
#define KEY_NUM_5       0xE5
#define KEY_NUM_6       0xE6
#define KEY_NUM_7       0xE7
#define KEY_NUM_8       0xE8
#define KEY_NUM_9       0xE9
#define KEY_NUM_SLASH   0xDC
#define KEY_NUM_ASTERISK 0xDD
#define KEY_NUM_MINUS   0xDE
#define KEY_NUM_PLUS    0xDF
#define KEY_NUM_ENTER   0xE0
#define KEY_NUM_PERIOD  0xEB

// Media keys (Consumer Control)
static const MediaKeyReport KEY_MEDIA_NEXT_TRACK = {1, 0};
static const MediaKeyReport KEY_MEDIA_PREVIOUS_TRACK = {2, 0};
static const MediaKeyReport KEY_MEDIA_STOP = {4, 0};
static const MediaKeyReport KEY_MEDIA_PLAY_PAUSE = {8, 0};
static const MediaKeyReport KEY_MEDIA_MUTE = {16, 0};
static const MediaKeyReport KEY_MEDIA_VOLUME_UP = {32, 0};
static const MediaKeyReport KEY_MEDIA_VOLUME_DOWN = {64, 0};
static const MediaKeyReport KEY_MEDIA_WWW_HOME = {128, 0};
static const MediaKeyReport KEY_MEDIA_LOCAL_MACHINE_BROWSER = {0, 1};
static const MediaKeyReport KEY_MEDIA_CALCULATOR = {0, 2};
static const MediaKeyReport KEY_MEDIA_WWW_BOOKMARKS = {0, 4};
static const MediaKeyReport KEY_MEDIA_WWW_SEARCH = {0, 8};
static const MediaKeyReport KEY_MEDIA_WWW_STOP = {0, 16};
static const MediaKeyReport KEY_MEDIA_WWW_BACK = {0, 32};
static const MediaKeyReport KEY_MEDIA_CONSUMER_CONTROL_CONFIGURATION = {0, 64};
static const MediaKeyReport KEY_MEDIA_EMAIL_READER = {0, 128};

// Standard letter/number keys (raw USB HID codes, not used directly)
#define KEY_A 0x04
#define KEY_B 0x05
#define KEY_C 0x06
#define KEY_D 0x07
#define KEY_E 0x08
#define KEY_F 0x09
#define KEY_G 0x0A
#define KEY_H 0x0B
#define KEY_I 0x0C
#define KEY_J 0x0D
#define KEY_K 0x0E
#define KEY_L 0x0F
#define KEY_M 0x10
#define KEY_N 0x11
#define KEY_O 0x12
#define KEY_P 0x13
#define KEY_Q 0x14
#define KEY_R 0x15
#define KEY_S 0x16
#define KEY_T 0x17
#define KEY_U 0x18
#define KEY_V 0x19
#define KEY_W 0x1A
#define KEY_X 0x1B
#define KEY_Y 0x1C
#define KEY_Z 0x1D
