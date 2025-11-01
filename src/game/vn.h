#ifndef VN_H__
#define VN_H__

#include "base/base_inc.h"
#include "core/core_inc.h"

// NOTE: If we did not have a dialog or an effect it should inherit
// the prev dialog right? the parent's.


typedef struct {
  buf name;
  u32 img_idx;
  buf text;
} VN_Active_Dialog;

typedef struct {
  buf text;
  u32 next_node_idx;
} VN_Choice; 

typedef struct {
  Effect e;
  v4 param0;
  v4 param1;
  //bool remove_on_exit;
  f32 duration;
} VN_Effect;

typedef struct {
  Json_Element *root_node;
  Json_Element *current_node;
  u32 next_node_idx;

  f32 duration;
  f32 duration_target; // if its zero, means no static transition

  VN_Active_Dialog active_dialog;

#define MAX_VN_CHOICE 4
  VN_Choice choices[MAX_VN_CHOICE];
  s32 choice_count;

  VN_Effect active_effect;
} VN_System;


VN_System vn_load_new_game(Arena *arena);

void vn_simulate(VN_System *vns, v2 screen_dim, f64 dt);


#endif 
