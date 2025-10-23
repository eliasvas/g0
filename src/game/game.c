#include "game.h"
#include "gui/gui.h"

// Forward Declarations, I don't like this.. @FIXME
typedef struct { u8 *data; u64 width; u64 height; } Platform_Image_Data;
Platform_Image_Data platform_load_image_bytes_as_rgba(const char *filepath);
f64 platform_get_time();

void game_init(Game_State *gs) {
  ogl_init(); // To create the bullshit empty VAO opengl side, nothing else

  gs->red = ogl_tex_make((u8[]){250,90,72,255}, 1,1, OGL_TEX_FORMAT_RGBA8U, (Ogl_Tex_Params){.wrap_s = OGL_TEX_WRAP_MODE_REPEAT});

  Platform_Image_Data image = platform_load_image_bytes_as_rgba("data/microgue.png");
  assert(image.width > 0);
  assert(image.height > 0);
  assert(image.data != nullptr);
  gs->atlas = ogl_tex_make(image.data, image.width, image.height, OGL_TEX_FORMAT_RGBA8U, (Ogl_Tex_Params){.wrap_s = OGL_TEX_WRAP_MODE_REPEAT, .wrap_t = OGL_TEX_WRAP_MODE_REPEAT});
  gs->font = font_util_load_default_atlas(gs->frame_arena, 64, 1024, 1024);


  gs->fill_effect = effect_make(EFFECT_KIND_FILL);
  gs->vortex_effect = effect_make(EFFECT_KIND_VORTEX);

  // Gui Stuff
  gui_context_init(gs->frame_arena, &gs->font);
  // DUMMY gui panel hierarchy for testing
  Gui_Panel *c1 = arena_push_array(gui_get_ctx()->persistent_arena, Gui_Panel, 1);
  c1->label = "c1";
  c1->parent_pct = 0.4;
  c1->split_axis = GUI_AXIS_Y;
  dll_push_back(gui_get_ctx()->root_panel->first, gui_get_ctx()->root_panel->last, c1);
  c1->parent = gui_get_ctx()->root_panel;
  assert(c1->parent == gui_get_ctx()->root_panel);

  Gui_Panel *c1u = arena_push_array(gui_get_ctx()->persistent_arena, Gui_Panel, 1);
  c1u->label = "c1u";
  c1u->parent_pct = 0.2;
  c1u->split_axis = GUI_AXIS_Y;
  dll_push_back(c1->first, c1->last, c1u);
  c1u->parent = c1;

  Gui_Panel *c1d = arena_push_array(gui_get_ctx()->persistent_arena, Gui_Panel, 1);
  c1d->label = "c1d";
  c1d->parent_pct = 0.8;
  c1d->split_axis = GUI_AXIS_Y;
  dll_push_back(c1->first, c1->last, c1d);
  c1d->parent = c1;

  Gui_Panel *c2 = arena_push_array(gui_get_ctx()->persistent_arena, Gui_Panel, 1);
  c2->label = "c2";
  c2->parent_pct = 0.6;
  c2->split_axis = GUI_AXIS_X;
  dll_push_back(gui_get_ctx()->root_panel->first, gui_get_ctx()->root_panel->last, c2);
  c2->parent = gui_get_ctx()->root_panel;
  assert(c1->next == c2);
  assert(c1->parent == gui_get_ctx()->root_panel);
  assert(c2->parent == gui_get_ctx()->root_panel);
  // ---------------------------------





  // Json library testing
  Json_Element *root = json_parse(gs->frame_arena, test_str);
  assert(root);

  Json_Element *e = json_lookup(root, MAKE_STR("msg-from"));
  assert(e);
  Json_Element *e2 = json_lookup(e, MAKE_STR("class"));
  assert(e2);
  assert(str_cmp("soldier", e2->value.data, str_len("soldier")));

  Json_Element *r = json_lookup(root, MAKE_STR("msg-nums"));
  assert(r);
  assert(buf_to_int(r->first->value) == 12);

  Json_Element *r2 = json_lookup(root, MAKE_STR("msg-floats"));
  assert(r2);
  assert(equalf(buf_to_float(r2->first->next->value), 5.22, 0.1));

  Json_Element *r3 = json_lookup(root, MAKE_STR("msg-bools"));
  assert(r3);
  assert(buf_to_bool(r3->first->value) == true);
}

void game_update(Game_State *gs, float dt) {
  Gui_Box *right = gui_box_lookup_from_key(0, gui_key_from_str("panel_c2"));
  if (!gui_box_is_nil(right)) {
    gs->game_viewport = right->r; 
  }
}

void game_render(Game_State *gs, float dt) {
  ogl_clear(col(0.0,0.0,0.0,1.0));
  gui_frame_begin(gs->screen_dim, dt);

  Gui_Box *leftup = gui_box_lookup_from_key(0, gui_key_from_str("panel_c1u"));
  assert(!gui_box_is_nil(leftup));
  gui_push_parent(leftup);
  // FIXME: These char[] are stack variables, we shouldn't do this..
  char fps_name[64];
  sprintf(fps_name, "fps: %f", 1.0/dt); // TODO: make an API that will not consider %% arguments, so we CAN animate them
  gui_set_next_bg_color(v4m(0.4,0.3,0.2,1));
  gui_set_next_pref_size(GUI_AXIS_X, (Gui_Size){.kind = GUI_SIZE_KIND_PARENT_PCT, 1.0, 0.0});
  gui_set_next_pref_size(GUI_AXIS_Y, (Gui_Size){.kind = GUI_SIZE_KIND_PARENT_PCT, 1.0, 0.0});
  gui_label(fps_name);

  v2 mp = input_get_mouse_pos();
  char mp_name[64];
  sprintf(mp_name, "mp: (%.0f,%.0f)", mp.x, mp.y); // TODO: make an API that will not consider %% arguments, so we CAN animate them
  gui_set_next_bg_color(v4m(0.4,0.2,0.3,1));
  gui_set_next_pref_size(GUI_AXIS_X, (Gui_Size){.kind = GUI_SIZE_KIND_PARENT_PCT, 1.0, 0.0});
  gui_set_next_pref_size(GUI_AXIS_Y, (Gui_Size){.kind = GUI_SIZE_KIND_PARENT_PCT, 1.0, 0.0});
  gui_label(mp_name);

  char pers_name[64];
  sprintf(pers_name, "allocd: %lu bytes", gs->persistent_arena->committed); // TODO: make an API that will not consider %% arguments, so we CAN animate them
  gui_set_next_bg_color(v4m(0.2,0.3,0.4,1));
  gui_set_next_pref_size(GUI_AXIS_X, (Gui_Size){.kind = GUI_SIZE_KIND_PARENT_PCT, 1.0, 0.0});
  gui_set_next_pref_size(GUI_AXIS_Y, (Gui_Size){.kind = GUI_SIZE_KIND_PARENT_PCT, 1.0, 0.0});
  gui_label(pers_name);

  gui_pop_parent();

  Gui_Box *leftdown = gui_box_lookup_from_key(0, gui_key_from_str("panel_c1d"));
  //leftdown->view_off.y = 20;
  assert(!gui_box_is_nil(leftdown));
  gui_set_next_parent(leftdown);

  static Gui_Scroll_Data sdata = {};
  Gui_Signal scroll_list = gui_scroll_list("scroll_list", GUI_AXIS_Y, &sdata);

  gui_push_parent(scroll_list.box);
  for (u32 i = 0; i < 10; i+=1) {
    gui_set_next_pref_size(GUI_AXIS_X, (Gui_Size){.kind = GUI_SIZE_KIND_PARENT_PCT, 1.0, 1.0});
    gui_set_next_pref_size(GUI_AXIS_Y, (Gui_Size){.kind = GUI_SIZE_KIND_PARENT_PCT, 1.0, 0.0});
    gui_set_next_bg_color(col(i * 0.1, 0.2, 0.4, 0.5));
    char name[64];
    sprintf(name, "button_%i", i);
    gui_button(name);
  }
  gui_pop_parent();

  gui_frame_end();


#define ATLAS_SPRITES_X 16
#define ATLAS_SPRITES_Y 10
  R2D* fs_rend = r2d_begin(gs->frame_arena, &(R2D_Cam){ .offset = v2m(0, 0), .origin = v2m(0,0), .zoom = 1.0, .rot_deg = 0.0, }, gs->game_viewport, gs->game_viewport);
  r2d_push_quad(fs_rend, (R2D_Quad) {
      .src_rect = rec(0,0,128,80),
      .dst_rect = rec(0,0,gs->game_viewport.w,gs->game_viewport.h),
      .c = col(1,1,1,1),
      .tex = gs->atlas,
  });
  r2d_end(fs_rend);

  // Effect Testing too :)
  Effect_Data vortex_data = {
    .screen_dim = v2m(gs->game_viewport.w, gs->game_viewport.h),
    .time_sec = platform_get_time(),
    .framerate = 1.0f/dt,
    .param0 = v4m(0.8,0,0,0), // param0.x is alpha currently
  };
  effect_render(&gs->vortex_effect, &vortex_data, gs->screen_dim, gs->game_viewport);

  float speedup = 3.0;
  f64 ts = platform_get_time()*speedup;
  R2D* rend = r2d_begin(gs->frame_arena, &(R2D_Cam){ .offset = v2m(gs->game_viewport.w/2.0, gs->game_viewport.h/2.0), .origin = v2m(5,5), .zoom = 30.0, .rot_deg = -ts,}, gs->game_viewport, gs->game_viewport);

  r2d_push_quad(rend, (R2D_Quad) {
      .src_rect = rec(0,0,0,0),
      .dst_rect = rec(0,0,10,10),
      .c = col(1,1,1,0.8),
      .tex = gs->red,
      .rot_deg = ts,
  });
  u32 xidx = (u32)ts % ATLAS_SPRITES_X;
  u32 yidx = (u32)ts / ATLAS_SPRITES_X;
  r2d_push_quad(rend, (R2D_Quad) {
      .src_rect = rec(xidx*8, yidx*8, 8, 8),
      .dst_rect = rec(1,1,8,8),
      .c = col(1,1,1,1),
      .tex = gs->atlas,
      .rot_deg = ts,
  });
  r2d_end(rend);

  /*
  // Effect Testing
  Effect_Data fill_data = {
    .viewport = gs->game_viewport,
    .time_sec = platform_get_time(),
    .framerate = 1.0f/dt,
    .param0 = v4m(0.1,0.1,0.9,0.1),
    .param1 = v4m(1,1,1,1),
  };
  effect_render(&gs->fill_effect, &fill_data);
  */
}

