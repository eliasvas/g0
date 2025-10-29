#ifndef VN_H__
#define VN_H__

#include "base/base_inc.h"
#include "core/core_inc.h"


typedef struct {
  Json_Element *root_node;
  Json_Element *current_node;

  f32 transition_sec; // Either text transition or effect transition
  f32 max_transition_sec;
  u32 img_idx;
  buf text; // Text to appear on text box
  buf name; // Character Name
} VN_System;


VN_System vn_load_new_game(Arena *arena);

void vn_simulate(VN_System *vns, f64 dt);


#endif 
