#include "game.h"
#include "gui/gui.h"

//struct platform_api { f64 (*get_time)(); };

void game_init(Game_State *gs) {
  ogl_init(); // To create the bullshit empty VAO opengl side, nothing else

  // Should these (for now) happen in platform.h?
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
  Gui_Panel *p_up = arena_push_array(gui_get_ctx()->persistent_arena, Gui_Panel, 1);
  p_up->label = MAKE_STR("p_up");
  p_up->parent_pct = 0.7;
  p_up->split_axis = GUI_AXIS_X;
  dll_push_back(gui_get_ctx()->root_panel->first, gui_get_ctx()->root_panel->last, p_up);
  p_up->parent = gui_get_ctx()->root_panel;


  Gui_Panel *p_down = arena_push_array(gui_get_ctx()->persistent_arena, Gui_Panel, 1);
  p_down->label = MAKE_STR("p_down");
  p_down->parent_pct = 0.3;
  p_down->split_axis = GUI_AXIS_Y;
  dll_push_back(gui_get_ctx()->root_panel->first, gui_get_ctx()->root_panel->last, p_down);
  p_down->parent = gui_get_ctx()->root_panel;

  Gui_Panel *p_up_left = arena_push_array(gui_get_ctx()->persistent_arena, Gui_Panel, 1);
  p_up_left->label = MAKE_STR("p_up_left");
  p_up_left->parent_pct = 0.2;
  p_up_left->split_axis = GUI_AXIS_Y;
  dll_push_back(p_up->first, p_up->last, p_up_left);
  p_up_left->parent = p_up;

  Gui_Panel *p_up_right = arena_push_array(gui_get_ctx()->persistent_arena, Gui_Panel, 1);
  p_up_right->label = MAKE_STR("p_up_right");
  p_up_right->parent_pct = 0.8;
  p_up_right->split_axis = GUI_AXIS_Y;
  dll_push_back(p_up->first, p_up->last, p_up_right);
  p_up_right->parent = p_up;
  // ---------------------------------

  // Json library testing
  Json_Element *root = json_parse(gs->frame_arena, test_str);
  assert(root);

  Json_Element *e = json_lookup(root, MAKE_STR("msg-from"));
  assert(e);
  Json_Element *e2 = json_lookup(e, MAKE_STR("class"));
  assert(e2);
  assert(str_cmp("soldier", e2->value.data, cstr_len("soldier")));

  Json_Element *r = json_lookup(root, MAKE_STR("msg-nums"));
  assert(r);
  assert(buf_to_int(r->first->value) == 12);

  Json_Element *r2 = json_lookup(root, MAKE_STR("msg-floats"));
  assert(r2);
  assert(equalf(buf_to_float(r2->first->next->value), 5.22, 0.1));

  buf lstr = arena_sprintf(gs->frame_arena, "msg-%s", "bools");
  assert(buf_eq(MAKE_STR("msg-bools"), lstr));
  Json_Element *r3 = json_lookup(root, lstr);
  //Json_Element *r3 = json_lookup(root, MAKE_STR("msg-bools"));
  assert(r3);
  assert(buf_to_bool(r3->first->value) == true);


  /*
  game_json[sizeof(game_json)-1] = '\0'; // null teminate the embedded json
  Json_Element *gj = json_parse(gs->persistent_arena, (char*)game_json);
  assert(gj);
  Json_Element *gj0_id = json_lookup(gj->first, MAKE_STR("ID"));
  assert(gj0_id);
  assert(buf_to_int(gj0_id->value) == 0);
  Json_Element *gj1_id = json_lookup(gj->first->next, MAKE_STR("ID"));
  assert(gj1_id);
  assert(buf_to_int(gj1_id->value) == 1);
  */

  gs->vns = vn_load_new_game(gs->persistent_arena);

}

void game_update(Game_State *gs, float dt) {
  Gui_Box *up_right = gui_box_lookup_from_key(0, gui_key_from_str(MAKE_STR("panel_p_up_right")));
  if (!gui_box_is_nil(up_right)) {
    rect r = up_right->r; 
    gs->game_viewport = ogl_to_gl_rect(r, gs->screen_dim.y);
  }
}

void game_render(Game_State *gs, float dt) {

#define ATLAS_SPRITES_X 16
#define ATLAS_SPRITES_Y 10
  // Draw background atlas
  R2D_Cmd cmd = (R2D_Cmd){ .kind = R2D_CMD_KIND_SET_VIEWPORT, .r = gs->game_viewport };
  r2d_push_cmd(gs->frame_arena, &gs->cmd_list, cmd, 256);
  cmd = (R2D_Cmd){ .kind = R2D_CMD_KIND_SET_SCISSOR, .r = gs->game_viewport };
  r2d_push_cmd(gs->frame_arena, &gs->cmd_list, cmd, 256);
  cmd = (R2D_Cmd){ .kind = R2D_CMD_KIND_SET_CAMERA, .c = (R2D_Cam){ .offset = v2m(0, 0), .origin = v2m(0,0), .zoom = 1.0, .rot_deg = 0.0, } };
  r2d_push_cmd(gs->frame_arena, &gs->cmd_list, cmd, 256);
  R2D_Quad quad = (R2D_Quad) {
      .src_rect = rec(0,0,128,80),
      .dst_rect = rec(0,0,gs->game_viewport.w,gs->game_viewport.h),
      .c = col(1,1,1,1),
      .tex = gs->atlas,
  };
  cmd = (R2D_Cmd){ .kind = R2D_CMD_KIND_ADD_QUAD, .q = quad };
  r2d_push_cmd(gs->frame_arena, &gs->cmd_list, cmd, 256);


  // Draw Hero_Bg in middle
  float speedup = 3.0;
  f64 ts = platform_get_time()*speedup;
  cmd = (R2D_Cmd){ .kind = R2D_CMD_KIND_SET_CAMERA, .c = (R2D_Cam){ .offset = v2m(gs->game_viewport.w/2.0, gs->game_viewport.h/2.0), .origin = v2m(5,5), .zoom = 30.0, .rot_deg = -ts,} };
  r2d_push_cmd(gs->frame_arena, &gs->cmd_list, cmd, 256);
  R2D_Quad hero_bg = (R2D_Quad) {
      .src_rect = rec(0,0,0,0),
      .dst_rect = rec(0,0,10,10),
      .c = col(1,1,1,0.8),
      .tex = gs->red,
      .rot_deg = ts,
  };
  cmd = (R2D_Cmd){ .kind = R2D_CMD_KIND_ADD_QUAD, .q = hero_bg };
  r2d_push_cmd(gs->frame_arena, &gs->cmd_list, cmd, 256);

  // Draw Hero in middle
  u32 tc = gs->vns.active_dialog.img_idx;
  u32 xidx = (u32)tc % ATLAS_SPRITES_X;
  u32 yidx = (u32)tc / ATLAS_SPRITES_X;
  R2D_Quad hero = (R2D_Quad) {
      .src_rect = rec(xidx*8, yidx*8, 8, 8),
      .dst_rect = rec(1,1,8,8),
      .c = col(1,1,1,1),
      .tex = gs->atlas,
      .rot_deg = ts,
  };
  cmd = (R2D_Cmd){ .kind = R2D_CMD_KIND_ADD_QUAD, .q = hero };
  r2d_push_cmd(gs->frame_arena, &gs->cmd_list, cmd, 256);




  gui_frame_begin(gs->screen_dim, &gs->input, &gs->cmd_list, dt);

  /*
  gui_push_font_scale(0.25);
  gui_push_text_alignment(GUI_TEXT_ALIGNMENT_LEFT);
  {
    Gui_Box *leftup = gui_box_lookup_from_key(0, gui_key_from_str(MAKE_STR("panel_c1u")));
    assert(!gui_box_is_nil(leftup));
    gui_push_parent(leftup);
    buf fps_name = arena_sprintf(gs->frame_arena, "fps: %f", 1.0/dt); 
    gui_set_next_text_color(col(0.9,0.9,0.3,1.0));
    gui_set_next_bg_color(v4m(0.4,0.3,0.2,1));
    gui_set_next_pref_size(GUI_AXIS_X, (Gui_Size){.kind = GUI_SIZE_KIND_PARENT_PCT, 1.0, 0.0});
    gui_set_next_pref_size(GUI_AXIS_Y, (Gui_Size){.kind = GUI_SIZE_KIND_PARENT_PCT, 1.0, 0.0});
    gui_label(fps_name);

    v2 mp = input_get_mouse_pos(&gs->input);
    buf mp_name = arena_sprintf(gs->frame_arena, "mp: (%.0f,%.0f)", mp.x, mp.y); 
    gui_set_next_text_color(col(0.7,0.7,0.7,1.0));
    gui_set_next_bg_color(v4m(0.4,0.2,0.3,1));
    gui_set_next_pref_size(GUI_AXIS_X, (Gui_Size){.kind = GUI_SIZE_KIND_PARENT_PCT, 1.0, 0.0});
    gui_set_next_pref_size(GUI_AXIS_Y, (Gui_Size){.kind = GUI_SIZE_KIND_PARENT_PCT, 1.0, 0.0});
    gui_label(mp_name);

    buf pers_name = arena_sprintf(gs->frame_arena, "allocd: %lu KB", gs->persistent_arena->committed/KB(1)); 
    gui_set_next_text_color(col(0.9,0.4,0.8,1.0));
    gui_set_next_bg_color(v4m(0.2,0.3,0.4,1));
    gui_set_next_pref_size(GUI_AXIS_X, (Gui_Size){.kind = GUI_SIZE_KIND_PARENT_PCT, 1.0, 0.0});
    gui_set_next_pref_size(GUI_AXIS_Y, (Gui_Size){.kind = GUI_SIZE_KIND_PARENT_PCT, 1.0, 0.0});
    gui_label(pers_name);

    buf tpers_name = arena_sprintf(gs->frame_arena, "tallocd: %lu KB", gs->frame_arena->committed/KB(1)); 
    gui_set_next_text_color(col(1.0,0.3,1.0,1.0));
    gui_set_next_bg_color(v4m(0.2,0.2,0.4,1));
    gui_set_next_pref_size(GUI_AXIS_X, (Gui_Size){.kind = GUI_SIZE_KIND_PARENT_PCT, 1.0, 0.0});
    gui_set_next_pref_size(GUI_AXIS_Y, (Gui_Size){.kind = GUI_SIZE_KIND_PARENT_PCT, 1.0, 0.0});
    gui_label(tpers_name);

    gui_pop_parent();
  }
  gui_pop_text_alignment();
  gui_pop_font_scale();
  */

  // Label List on up-left tile of screen
  Gui_Box *up_left = gui_box_lookup_from_key(0, gui_key_from_str(MAKE_STR("panel_p_up_left")));
  assert(!gui_box_is_nil(up_left));
  gui_set_next_parent(up_left);

  static Gui_Scroll_Data sdata = {
    .scroll_percent = 0.0,
    .item_px = 30,
    .item_count = 4,
    .scroll_bar_px = 20,
    .scroll_button_px = 40,
    .scroll_button_color = (color){{0.6,0.2,0.2,1}},
    .scroll_speed = 0.5,
  };
  if (input_mkey_pressed(&gs->input, INPUT_MOUSE_RMB))sdata.item_count+=1;
  gui_push_font_scale(0.4);
  gui_scroll_list_begin(MAKE_STR("scroll_list"), GUI_AXIS_Y, &sdata);
  for (s32 i = 0; i < sdata.item_count; i+=1) {
    gui_set_next_bg_color(col(i * 0.1, 0.2, 0.4, 0.5));
    buf name = arena_sprintf(gs->frame_arena, "button##_%i", i); 
    gui_button(name);
  }
  gui_scroll_list_end(MAKE_STR("scroll_list"));
  gui_pop_font_scale();

  // This updates ui
  vn_simulate(&gs->vns, gs->screen_dim, dt);

  gui_frame_end();


}
