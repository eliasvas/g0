#include "game.h"
#include "gui/gui.h"

// TODO: screen->world and world->screen with camera, so we can spawn at edges of camera entities
#define ATLAS_SPRITES_X 16
#define ATLAS_SPRITES_Y 10
#define TILE_W 8
#define TILE_H 8
void game_push_atlas_rect(Game_State *gs, u32 atlas_idx, rect r) {
  u32 xidx = (u32)atlas_idx % ATLAS_SPRITES_X;
  u32 yidx = (u32)atlas_idx / ATLAS_SPRITES_X;
  R2D_Quad quad = (R2D_Quad) {
      .src_rect = rec(xidx*TILE_W,yidx*TILE_H,TILE_W,TILE_H),
      .dst_rect = r,
      .c = col(1,1,1,1),
      .tex = gs->atlas,
      .rot_deg = 0,
  };
  R2D_Cmd cmd = (R2D_Cmd){ .kind = R2D_CMD_KIND_ADD_QUAD, .q = quad};
  r2d_push_cmd(gs->frame_arena, &gs->cmd_list, cmd, 256);
}

void game_push_atlas_rect_at(Game_State *gs, u32 atlas_idx, v2 pos) {
  game_push_atlas_rect(gs, atlas_idx, rec(pos.x-TILE_W/2,pos.y-TILE_H/2,TILE_W,TILE_H));
}


void game_init(Game_State *gs) {
  gui_context_init(gs->frame_arena, &gs->font);
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
  //cmd = (R2D_Cmd){ .kind = R2D_CMD_KIND_SET_CAMERA, .c = (R2D_Cam){ .offset = v2m(gs->game_viewport.w/2.0, gs->game_viewport.h/2.0), .origin = v2m(0,0), .zoom = 10.0, .rot_deg = 0} };
  cmd = (R2D_Cmd){ .kind = R2D_CMD_KIND_SET_CAMERA, .c = (R2D_Cam){ .offset = v2m(0,0), .origin = v2m(0,0), .zoom = 10.0, .rot_deg = 0} };
  r2d_push_cmd(gs->frame_arena, &gs->cmd_list, cmd, 256);
 
  /*
  // Draw some sample sprites
  game_push_atlas_rect_at(gs, 2, v2m(0,0));
  game_push_atlas_rect_at(gs, 3, v2m(8,3));
  game_push_atlas_rect_at(gs, 3, v2m(-8,3));
  game_push_atlas_rect_at(gs, 1, v2m(16,6));
  game_push_atlas_rect_at(gs, 1, v2m(-16,6));
  */

  // Draw The backgound
  u32 tilemap[9][17] = {
    {1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},
    {1, 1, 0, 0,  0, 1, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1},
    {1, 1, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 1, 0, 1},
    {1, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 1},
    {0, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 0},
    {1, 1, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 1, 0, 0, 1},
    {1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  1, 0, 0, 0, 1},
    {1, 1, 1, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1},
    {0, 0, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},
  };
  for (u32 row = 0; row < 9; row+=1) {
    for (u32 col = 0; col < 17; col+=1) {
      if (tilemap[row][col]) {
        game_push_atlas_rect(gs, 1, rec(col*TILE_W, row*TILE_H,TILE_W,TILE_H));
      } else {
        game_push_atlas_rect(gs, 69, rec(col*TILE_W, row*TILE_H,TILE_W,TILE_H));
      }
    }
  }

  // Move + Draw the hero
  f32 hero_speed = 100.0;
  if (input_key_down(&gs->input, KEY_SCANCODE_UP))gs->player_pos.y-=hero_speed*dt;
  if (input_key_down(&gs->input, KEY_SCANCODE_DOWN))gs->player_pos.y+=hero_speed*dt;
  if (input_key_down(&gs->input, KEY_SCANCODE_LEFT))gs->player_pos.x-=hero_speed*dt;
  if (input_key_down(&gs->input, KEY_SCANCODE_RIGHT))gs->player_pos.x+=hero_speed*dt;
  game_push_atlas_rect_at(gs, 9, gs->player_pos);



  // In the end, perform a UI pass (TBA)
  gui_frame_begin(gs->screen_dim, &gs->input, &gs->cmd_list, dt);
  gui_set_next_bg_color(col(0.1, 0.2, 0.4, 0.5));
  Gui_Signal bs = gui_button(MAKE_STR("Exit"));
  if (bs.flags & GUI_SIGNAL_FLAG_LMB_PRESSED)printf("Exit pressed!\n");
  gui_frame_end();
}

void game_shutdown(Game_State *gs) { }
