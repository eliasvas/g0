#ifndef INPUT_H__
#define INPUT_H__
#include "helper.h"
#include "math3d.h"

typedef enum {
  INPUT_EVENT_KIND_KEEB,
  INPUT_EVENT_KIND_MOUSE,
  INPUT_EVENT_KIND_GAMEPAD,
} Input_Event_Kind;

typedef enum {
  INPUT_MOUSE_LMB,
  INPUT_MOUSE_MMB,
  INPUT_MOUSE_RMB,
  INPUT_MOUSE_COUNT,
} Input_Mouse_Button;

typedef struct {
  b32 button_state [INPUT_MOUSE_COUNT];
  v2 mouse_pos;
  // scroll stuff
} Input_Mouse_Event;

typedef enum {
    KEY_SCANCODE_UNKNOWN = 0,
    KEY_SCANCODE_A = 4,
    KEY_SCANCODE_B = 5,
    KEY_SCANCODE_C = 6,
    KEY_SCANCODE_D = 7,
    KEY_SCANCODE_E = 8,
    KEY_SCANCODE_F = 9,
    KEY_SCANCODE_G = 10,
    KEY_SCANCODE_H = 11,
    KEY_SCANCODE_I = 12,
    KEY_SCANCODE_J = 13,
    KEY_SCANCODE_K = 14,
    KEY_SCANCODE_L = 15,
    KEY_SCANCODE_M = 16,
    KEY_SCANCODE_N = 17,
    KEY_SCANCODE_O = 18,
    KEY_SCANCODE_P = 19,
    KEY_SCANCODE_Q = 20,
    KEY_SCANCODE_R = 21,
    KEY_SCANCODE_S = 22,
    KEY_SCANCODE_T = 23,
    KEY_SCANCODE_U = 24,
    KEY_SCANCODE_V = 25,
    KEY_SCANCODE_W = 26,
    KEY_SCANCODE_X = 27,
    KEY_SCANCODE_Y = 28,
    KEY_SCANCODE_Z = 29,

    KEY_SCANCODE_1 = 30,
    KEY_SCANCODE_2 = 31,
    KEY_SCANCODE_3 = 32,
    KEY_SCANCODE_4 = 33,
    KEY_SCANCODE_5 = 34,
    KEY_SCANCODE_6 = 35,
    KEY_SCANCODE_7 = 36,
    KEY_SCANCODE_8 = 37,
    KEY_SCANCODE_9 = 38,
    KEY_SCANCODE_0 = 39,

    KEY_SCANCODE_RETURN = 40,
    KEY_SCANCODE_ESCAPE = 41,
    KEY_SCANCODE_BACKSPACE = 42,
    KEY_SCANCODE_TAB = 43,
    KEY_SCANCODE_SPACE = 44,

    KEY_SCANCODE_MINUS = 45,
    KEY_SCANCODE_EQUALS = 46,
    KEY_SCANCODE_LEFTBRACKET = 47,
    KEY_SCANCODE_RIGHTBRACKET = 48,
    KEY_SCANCODE_BACKSLASH = 49,
    KEY_SCANCODE_NONUSHASH = 50,
    KEY_SCANCODE_SEMICOLON = 51,
    KEY_SCANCODE_APOSTROPHE = 52,
    KEY_SCANCODE_GRAVE = 53,
    KEY_SCANCODE_COMMA = 54,
    KEY_SCANCODE_PERIOD = 55,
    KEY_SCANCODE_SLASH = 56,
    KEY_SCANCODE_CAPSLOCK = 57,

    KEY_SCANCODE_F1 = 58,
    KEY_SCANCODE_F2 = 59,
    KEY_SCANCODE_F3 = 60,
    KEY_SCANCODE_F4 = 61,
    KEY_SCANCODE_F5 = 62,
    KEY_SCANCODE_F6 = 63,
    KEY_SCANCODE_F7 = 64,
    KEY_SCANCODE_F8 = 65,
    KEY_SCANCODE_F9 = 66,
    KEY_SCANCODE_F10 = 67,
    KEY_SCANCODE_F11 = 68,
    KEY_SCANCODE_F12 = 69,

    KEY_SCANCODE_PRINTSCREEN = 70,
    KEY_SCANCODE_SCROLLLOCK = 71,
    KEY_SCANCODE_PAUSE = 72,
    KEY_SCANCODE_INSERT = 73,
    KEY_SCANCODE_HOME = 74,
    KEY_SCANCODE_PAGEUP = 75,
    KEY_SCANCODE_DELETE = 76,
    KEY_SCANCODE_END = 77,
    KEY_SCANCODE_PAGEDOWN = 78,
    KEY_SCANCODE_RIGHT = 79,
    KEY_SCANCODE_LEFT = 80,
    KEY_SCANCODE_DOWN = 81,
    KEY_SCANCODE_UP = 82,
    KEY_SCANCODE_COUNT,
} Key_Scancode;

typedef struct {
  Key_Scancode scancode;
  b32 is_down;
} Input_Keeb_Event;

// TODO: this
typedef struct {
  u32 TBH;
} Input_Gamepad_Event;

typedef struct {
  union {
    Input_Mouse_Event   me;
    Input_Keeb_Event    ke;
    Input_Gamepad_Event ge;
  } data;
  Input_Event_Kind kind;
} Input_Event;

typedef struct Input_Event_Node Input_Event_Node;
struct Input_Event_Node {
  Input_Event_Node *next;
  Input_Event evt;
};

typedef struct {
    b32 keyboard_state[KEY_SCANCODE_COUNT];
    b32 mouse_state[INPUT_MOUSE_COUNT];

    Input_Event_Node *first;
    Input_Event_Node *last;

    v2 mouse_pos;
    v2 prev_mouse_pos;
} Input_Singleton;

Input_Singleton input;

b32 input_key_down(Key_Scancode key) {
  return input.keyboard_state[key];
}

b32 input_mkey_down(Input_Mouse_Button key) {
  return input.mouse_state[key];
}

b32 input_key_up(Key_Scancode key) {
  return input.keyboard_state[key];
}

b32 input_mkey_up(Input_Mouse_Button key) {
  return input.mouse_state[key];
}

v2 input_get_mousepos() {
  return input.mouse_pos;
}

#endif
