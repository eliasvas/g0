#ifndef _GAME_H__
#define _GAME_H__

#include "vn.h"
#include "base/base_inc.h"
#include "core/core_inc.h"

typedef struct {
  Arena *persistent_arena; // For persistent allocations
  Arena *frame_arena; // For per-frame allocations
  rect game_viewport;
  
  // Interface between platform <-> game
  f32 time_sec;
  v2 screen_dim;
  Input input;
  R2D_Cmd_Chunk_List cmd_list;

  // Loaded Asset resources (TODO: Asset system)
  Ogl_Tex atlas;
  Ogl_Tex red;
  Font_Info font;
  Effect fill_effect;
  Effect vortex_effect;

  // Why is this here
  VN_System vns;
} Game_State;

void game_init(Game_State *gs);
void game_update(Game_State *gs, f32 dt);
void game_render(Game_State *gs, f32 dt);
void game_shutdown(Game_State *gs);



typedef void (*game_init_fn) (Game_State *gs);
typedef void (*game_update_fn) (Game_State *gs, f32 dt);
typedef void (*game_render_fn) (Game_State *gs, f32 dt);
typedef void (*game_shutdown_fn) (Game_State *gs);

typedef struct {
  game_init_fn init;
  game_update_fn update;
  game_render_fn render;
  game_shutdown_fn shutdown;

  void *lib;
  s64 last_modified;

  u64 api_version;
} Game_Api;

#endif
