#ifndef _GAME_H__
#define _GAME_H__

#include "vn.h"
#include "base/base_inc.h"
#include "core/core_inc.h"

typedef struct {
  Arena *persistent_arena; // For persistent allocations
  Arena *frame_arena; // For per-frame allocations
  v2 screen_dim;

  Input input;

  rect game_viewport;

  Ogl_Tex atlas;
  Ogl_Tex red;

  Font_Info font;

  Effect fill_effect;
  Effect vortex_effect;

  VN_System vns;
} Game_State;

// Not sure if these should be exposed as we will load them via DLL in the future
void game_init(Game_State *gs);
void game_update(Game_State *gs, f32 dt);
void game_render(Game_State *gs, f32 dt);

#endif
