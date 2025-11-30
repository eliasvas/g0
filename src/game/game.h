#ifndef _GAME_H__
#define _GAME_H__

#include "base/base_inc.h"
#include "core/core_inc.h"

typedef union iv2 {
    struct { s32 x,y; };
    struct { s32 u,v; };
    struct { s32 r,g; };
    s32 raw[2];
}iv2;
static iv2 iv2m(s32 x, s32 y)    { return (iv2){{x, y}}; }

typedef struct {
  iv2 tile_count; // how many tiles in each axis
  iv2 tile_dim; // how big each tile is
  iv2 upper_left_coords;
  u32 *tiles;
} Tile_Map;

typedef struct {
  iv2 tilemap_count;
  Tile_Map *maps;
} World;

typedef struct {
  Arena *persistent_arena; // For persistent allocations
  Arena *frame_arena; // For per-frame allocations
  rect game_viewport;
  
  // Interface between platform <-> game
  f32 time_sec;
  v2 screen_dim;
  Input input;
  R2D_Cmd_Chunk_List cmd_list;

  // Game specific stuff
  v2 player_pos;
  World world;
  
  // Loaded Asset resources (TODO: Asset system)
  Ogl_Tex atlas;
  Ogl_Tex red;
  Font_Info font;
  Effect fill_effect;
  Effect vortex_effect;

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
