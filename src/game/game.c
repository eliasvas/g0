#include "game.h"
#include "gui/gui.h"


// TODO: screen->world and world->screen with camera, so we can spawn at edges of camera entities
#define TILE_W 8
#define TILE_H 8


//-------------------------------------
// draw.h ??
#define ATLAS_SPRITES_X 16
#define ATLAS_SPRITES_Y 10
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
//-----------------------------------------------


u32 get_tilemap_value_nocheck(World *world, Tile_Map *tm, iv2 tile_coords) {
  assert(tm);
  assert((tile_coords.x >= 0) && (tile_coords.x < world->tile_dim_px.x) &&
        (tile_coords.y >= 0) && (tile_coords.y < world->tile_dim_px.y));

  return tm->tiles[tile_coords.x + world->tile_count.x*tile_coords.y];
}

b32 is_tilemap_point_empty(World *world, Tile_Map *tm, v2 test_point) {
  b32 empty = false;

  if (tm) {
    iv2 tile_pos = (iv2){
      .x = (s32)test_point.x,
      .y = (s32)test_point.y,
    };

    if ((tile_pos.x >=0) && (tile_pos.x < world->tile_count.x) && 
        (tile_pos.y >= 0) && (tile_pos.y < world->tile_count.y)) {
      u32 tm_value = get_tilemap_value_nocheck(world, tm, tile_pos);
      empty = (tm_value == 0);
    } 
  }

  return empty;
}

Tile_Map *get_tilemap(World *world, iv2 tilemap_idx) {
  Tile_Map *tm = nullptr;
  if ((tilemap_idx.x >= 0) && (tilemap_idx.x < world->tile_dim_px.x) &&
        (tilemap_idx.y >= 0) && (tilemap_idx.x < world->tile_dim_px.y)) {
    tm = &world->maps[tilemap_idx.x + world->tilemap_count.x * tilemap_idx.y];
  }
  return tm;
}

Canonical_Position get_canonical_position(World *world, Raw_Position pos) {
  Canonical_Position cpos = {};

  cpos.tile_coords = iv2m((s32)(pos.tile_coords.x), (s32)(pos.tile_coords.y));
  cpos.tile_rel_coords = v2m(
      pos.tile_coords.x - cpos.tile_coords.x,
      pos.tile_coords.y - cpos.tile_coords.y
  );

  // Transitions happen here
  cpos.tilemap_coords = pos.tilemap_coords;
  if (cpos.tile_coords.x >= world->tile_count.x) {
    cpos.tilemap_coords.x += 1;
    cpos.tile_coords.x = 0;
  }
  if (cpos.tile_coords.x < 0) {
    cpos.tilemap_coords.x -= 1;
    cpos.tile_coords.x = world->tile_count.x-1;
  }
  if (cpos.tile_coords.y >= world->tile_count.y) {
    cpos.tilemap_coords.y += 1;
    cpos.tile_coords.y = 0;
  }
  if (cpos.tile_coords.y < 0) {
    cpos.tilemap_coords.y -= 1;
    cpos.tile_coords.y = world->tile_count.y-1;
  }

  return cpos;
}

b32 is_world_point_empty(World *world, Raw_Position pos) {
  b32 empty = false;

  Canonical_Position cpos = get_canonical_position(world, pos);
  Tile_Map *tm = get_tilemap(world, cpos.tilemap_coords);
  empty = is_tilemap_point_empty(world, tm, v2m(cpos.tile_coords.x, cpos.tile_coords.y));

  return empty;
}


void game_init(Game_State *gs) {
  // Initialize the gui
  gui_context_init(gs->frame_arena, &gs->font);

  // Initialize the world
  static u32 tilemap00[6][8] = {
    {1, 1, 1, 1,  1, 1, 1, 1},
    {1, 0, 0, 0,  0, 0, 0, 1},
    {1, 0, 1, 0,  0, 1, 0, 0},
    {1, 0, 0, 0,  0, 0, 0, 1},
    {1, 0, 0, 0,  0, 0, 0, 1},
    {1, 1, 1, 0,  0, 1, 1, 1},
  };

  static u32 tilemap10[6][8] = {
    {1, 1, 1, 1,  1, 1, 1, 1},
    {1, 0, 0, 0,  0, 0, 1, 1},
    {0, 0, 0, 0,  0, 0, 0, 1},
    {1, 0, 0, 0,  0, 0, 0, 1},
    {1, 0, 0, 0,  0, 0, 1, 1},
    {1, 1, 1, 0,  0, 1, 1, 1},
  };

  static u32 tilemap11[6][8] = {
    {1, 1, 1, 0,  0, 1, 1, 1},
    {1, 1, 1, 0,  0, 1, 1, 1},
    {0, 0, 0, 0,  0, 0, 0, 1},
    {1, 0, 0, 0,  0, 0, 0, 1},
    {1, 0, 0, 0,  0, 0, 0, 1},
    {1, 1, 1, 1,  1, 1, 1, 1},
  };

  static u32 tilemap01[6][8] = {
    {1, 1, 1, 0,  0, 1, 1, 1},
    {1, 1, 0, 0,  0, 0, 1, 1},
    {1, 0, 0, 0,  0, 0, 0, 0},
    {1, 0, 0, 0,  0, 0, 0, 1},
    {1, 1, 0, 0,  0, 0, 1, 1},
    {1, 1, 1, 1,  1, 1, 1, 1},
  };
  static Tile_Map maps[2][2] = {};

  gs->world = (World) {
    .tilemap_count = iv2m(2,2),
    .tile_dim_px = iv2m(8,8),
    .tile_count = iv2m(8,6),
    .maps = (Tile_Map*)maps,
  };

  maps[0][0].tiles = (u32*)tilemap00;
  maps[0][1].tiles = (u32*)tilemap10;
  maps[1][1].tiles = (u32*)tilemap11;
  maps[1][0].tiles = (u32*)tilemap01;


  // Initialize the player
  gs->player_pos = v2m(5*TILE_W,3*TILE_H);
  gs->player_dim = v2m(TILE_W*0.9,TILE_H*0.9);
  gs->player_tilemap = iv2m(0,0);
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

  Tile_Map *tm = get_tilemap(&gs->world, gs->player_tilemap);

  // Draw The backgound
  for (u32 row = 0; row < 6; row+=1) {
    for (u32 col = 0; col < 8; col+=1) {
      if (is_tilemap_point_empty(&gs->world, tm, v2m(col, row))) {
        game_push_atlas_rect(gs, 69, rec(col*TILE_W, row*TILE_H,TILE_W,TILE_H));
      } else {
        game_push_atlas_rect(gs, 1, rec(col*TILE_W, row*TILE_H,TILE_W,TILE_H));
      }
    }
  }

  // Move + Draw the hero
  v2 new_player_pos = gs->player_pos;
  f32 hero_speed = 100.0;
  if (input_key_down(&gs->input, KEY_SCANCODE_UP))new_player_pos.y-=hero_speed*dt;
  if (input_key_down(&gs->input, KEY_SCANCODE_DOWN))new_player_pos.y+=hero_speed*dt;
  if (input_key_down(&gs->input, KEY_SCANCODE_LEFT))new_player_pos.x-=hero_speed*dt;
  if (input_key_down(&gs->input, KEY_SCANCODE_RIGHT))new_player_pos.x+=hero_speed*dt;


  Raw_Position player_pos = {
    .tilemap_coords = gs->player_tilemap,
    .tile_coords = v2_divf(v2_add(new_player_pos, v2m(0, gs->player_dim.y/2)), TILE_W),
  };
  Raw_Position player_pos_left = player_pos;
  player_pos_left.tile_coords.x -= (gs->player_dim.x/2) / TILE_W;
  Raw_Position player_pos_right = player_pos;
  player_pos_right.tile_coords.x += (gs->player_dim.x/2) / TILE_W;

  if (is_world_point_empty(&gs->world, player_pos) && 
      is_world_point_empty(&gs->world, player_pos_left) &&
      is_world_point_empty(&gs->world, player_pos_right)) {
    Canonical_Position cpos = get_canonical_position(&gs->world, player_pos);

    gs->player_tilemap = cpos.tilemap_coords;

    gs->player_pos = v2_multf(v2_add(cpos.tile_rel_coords, v2m(cpos.tile_coords.x, cpos.tile_coords.y)), TILE_W);
    gs->player_pos.y -= gs->player_dim.y/2; // because our playerpos is in the center..
    //gs->player_pos = new_player_pos;
  }
  game_push_atlas_rect(gs, 9, rec(gs->player_pos.x - gs->player_dim.x/2, gs->player_pos.y - gs->player_dim.y/2, gs->player_dim.x, gs->player_dim.y));


  // ..
  // ..
  // In the end, perform a UI pass (TBA)
  gui_frame_begin(gs->screen_dim, &gs->input, &gs->cmd_list, dt);
  gui_set_next_bg_color(col(0.1, 0.2, 0.4, 0.5));
  Gui_Signal bs = gui_button(MAKE_STR("Exit"));
  if (bs.flags & GUI_SIGNAL_FLAG_LMB_PRESSED)printf("Exit pressed!\n");
  gui_frame_end();
}

void game_shutdown(Game_State *gs) { }
