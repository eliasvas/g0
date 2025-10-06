#include "input.h"

// TODO: Maybe we shouuld have an Input_Singleton inside Game_State, NO GLOBALS!
static Input_Singleton g_input;

// TODO: Gamepad Events
// TODO: Scrolling Events

void g_input_process_events() {
  for (Input_Event_Node *node = g_input.first; node != nullptr; node = node->next) {
    Input_Event evt = node->evt;
    switch (evt.kind) {
      case INPUT_EVENT_KIND_NONE:
        // We should never have empty events in the queue
        assert(0 && "Empty event cannot be processed");
        break;
      case INPUT_EVENT_KIND_KEEB:
        Input_Keeb_Event ke = evt.data.ke;
        if (g_input.keeb_state[ke.scancode].is_down != ke.is_down) {
          g_input.keeb_state[ke.scancode].is_down = ke.is_down;
          g_input.keeb_state[ke.scancode].transition_count += 1;
        }
        break;
      case INPUT_EVENT_KIND_MOUSE:
        Input_Mouse_Event me = evt.data.me;
        if (g_input.mouse_state[me.button].is_down != me.is_down) {
          g_input.mouse_state[me.button].is_down = me.is_down;
          g_input.mouse_state[me.button].transition_count += 1;
        }
        break;
      case INPUT_EVENT_KIND_MOUSEMOTION:
        g_input.mouse_pos = evt.data.mme.mouse_pos; 
        break;
      case INPUT_EVENT_KIND_GAMEPAD:
        // TBH
        break;
      default: break;
    }
  }
  // clear the event queue at the end
  g_input.first = nullptr;
  g_input.last = nullptr;
}

void g_input_end_frame() {
  // Reset transition counts and set 'was_down' to previous frame's is_down field
  for (u32 keeb_key_idx = 0; keeb_key_idx < KEY_SCANCODE_COUNT; ++keeb_key_idx) {
    g_input.keeb_state[keeb_key_idx].was_down = g_input.keeb_state[keeb_key_idx].is_down;
    g_input.keeb_state[keeb_key_idx].transition_count = 0;
  }
  for (u32 mouse_key_idx = 0; mouse_key_idx < KEY_SCANCODE_COUNT; ++mouse_key_idx) {
    g_input.mouse_state[mouse_key_idx].was_down = g_input.mouse_state[mouse_key_idx].is_down;
    g_input.mouse_state[mouse_key_idx].transition_count = 0;
  }
  g_input.prev_mouse_pos = g_input.mouse_pos;
}

void g_input_push_event(Arena *arena, Input_Event *evt) {
  if (evt->kind != INPUT_EVENT_KIND_NONE) {
    Input_Event_Node *new_event = arena_push_array(arena, Input_Event_Node, 1);
    M_COPY(&new_event->evt, evt, sizeof(Input_Event));
    sll_queue_push(g_input.first, g_input.last, new_event);
  }
}

// TODO: Maybe we should also take transitions into account? (yes)

b32 input_key_pressed(Key_Scancode key) {
  return g_input.keeb_state[key].is_down && !g_input.keeb_state[key].was_down;
}
b32 input_key_released(Key_Scancode key) {
  return !g_input.keeb_state[key].is_down && g_input.keeb_state[key].was_down;
}
b32 input_key_up(Key_Scancode key) {
  return !g_input.keeb_state[key].is_down;
}
b32 input_key_down(Key_Scancode key) {
  return g_input.keeb_state[key].is_down;
}

b32 input_mkey_pressed(Input_Mouse_Button button) {
  return g_input.mouse_state[button].is_down && !g_input.mouse_state[button].was_down;
}
b32 input_mkey_released(Input_Mouse_Button button) {
  return !g_input.mouse_state[button].is_down && g_input.mouse_state[button].was_down;
}
b32 input_mkey_up(Input_Mouse_Button button) {
  return !g_input.mouse_state[button].is_down;
}
b32 input_mkey_down(Input_Mouse_Button button) {
  return g_input.mouse_state[button].is_down;
}

v2 input_get_mouse_pos() {
  return g_input.mouse_pos;
}
v2 input_get_mouse_delta() {
  return v2_sub(g_input.mouse_pos, g_input.prev_mouse_pos);
}

