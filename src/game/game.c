#include "game.h"
#include "gui/gui.h"

#define ATLAS_SPRITES_X 16
#define ATLAS_SPRITES_Y 10
void game_push_atlas_rect(Game_State *gs, u32 atlas_idx, rect r) {
  u32 xidx = (u32)atlas_idx % ATLAS_SPRITES_X;
  u32 yidx = (u32)atlas_idx / ATLAS_SPRITES_X;
  R2D_Quad quad = (R2D_Quad) {
      .src_rect = rec(xidx*8,yidx*8,8,8),
      .dst_rect = r,
      .c = col(1,1,1,1),
      .tex = gs->atlas,
      .rot_deg = 0,
  };
  R2D_Cmd cmd = (R2D_Cmd){ .kind = R2D_CMD_KIND_ADD_QUAD, .q = quad};
  r2d_push_cmd(gs->frame_arena, &gs->cmd_list, cmd, 256);
}

void game_push_atlas_rect_at(Game_State *gs, u32 atlas_idx, v2 pos) {
  game_push_atlas_rect(gs, atlas_idx, rec(pos.x-4,pos.y-4,8,8));
}


void game_init(Game_State *gs) {
  //gui_context_init(gs->frame_arena, &gs->font);
}

void game_update(Game_State *gs, float dt) {
  gs->game_viewport = rec(0,0,gs->screen_dim.x, gs->screen_dim.y);
}


void game_render(Game_State *gs, float dt) {
  // Push viewport, scissor and camera (we will not change these the whole frame except in UI pass)
  R2D_Cmd cmd = (R2D_Cmd){ .kind = R2D_CMD_KIND_SET_VIEWPORT, .r = gs->game_viewport };
  r2d_push_cmd(gs->frame_arena, &gs->cmd_list, cmd, 256);
  cmd = (R2D_Cmd){ .kind = R2D_CMD_KIND_SET_SCISSOR, .r = gs->game_viewport };
  r2d_push_cmd(gs->frame_arena, &gs->cmd_list, cmd, 256);
  cmd = (R2D_Cmd){ .kind = R2D_CMD_KIND_SET_CAMERA, .c = (R2D_Cam){ .offset = v2m(gs->game_viewport.w/2.0, gs->game_viewport.h/2.0), .origin = v2m(0,0), .zoom = 10.0, .rot_deg = 0} };
  r2d_push_cmd(gs->frame_arena, &gs->cmd_list, cmd, 256);
 
  f32 hero_speed = 50.0;
  static v2 pos;
  if (input_key_down(&gs->input, KEY_SCANCODE_UP))pos.y-=hero_speed*dt;
  if (input_key_down(&gs->input, KEY_SCANCODE_DOWN))pos.y+=hero_speed*dt;
  if (input_key_down(&gs->input, KEY_SCANCODE_LEFT))pos.x-=hero_speed*dt;
  if (input_key_down(&gs->input, KEY_SCANCODE_RIGHT))pos.x+=hero_speed*dt;
  game_push_atlas_rect_at(gs, 9, pos);

  // In the end, perform a UI pass (TBA)
  //gui_frame_begin(gs->screen_dim, &gs->input, &gs->cmd_list, dt);
  //gui_frame_end();
}

void game_shutdown(Game_State *gs) { }
