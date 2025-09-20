#ifndef _GAME_H__
#define _GAME_H__
#include "helper.h"
#include "arena.h"
#include "math3d.h"

typedef struct {
  Arena *persistent_arena; // For persistent allocations
  Arena *frame_arena; // For per-frame allocations
  v2 screen_dim;
} Game_State;

// Not sure if these should be exposed as we will load them via DLL in the future
void game_init(Game_State *gs);
void game_update(Game_State *gs, float dt);
void game_render(Game_State *gs);

#endif
