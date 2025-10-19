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

}

void game_render(Game_State *gs, float dt) {
  ogl_clear((Ogl_Color){0.2,0.2,0.25,1.0});

#define ATLAS_SPRITES_X 16
#define ATLAS_SPRITES_Y 10
  R2D* fs_rend = r2d_begin(gs->frame_arena, &(R2D_Cam){ .offset = v2m(0, 0), .origin = v2m(0,0), .zoom = 1.0, .rot_deg = 0.0, }, gs->screen_dim);
  r2d_push_quad(fs_rend, (R2D_Quad) {
      .src_rect = (R2D_Rect){0,0,128,80},
      .dst_rect = (R2D_Rect){0,0,gs->screen_dim.x,gs->screen_dim.y},
      .color = (R2D_Color){1,1,1,1},
      .tex = gs->atlas,
  });
  r2d_end(fs_rend);

  // Effect Testing too :)
  Effect_Data vortex_data = {
    .screen_dim = gs->screen_dim,
    .time_sec = platform_get_time(),
    .framerate = 1.0f/dt,
    .param0 = v4m(0.8,0,0,0), // param0.x is alpha currently
  };
  effect_render(&gs->vortex_effect, &vortex_data);

  char text_to_draw[64];
  f32 fps = 1.0/dt;
  f32 font_scale = 0.5;
  sprintf(text_to_draw, "fps: %.2f", fps);
  f32 w = font_util_measure_text_width(&gs->font, text_to_draw, font_scale);
  font_util_debug_draw_text(&gs->font, gs->frame_arena, gs->screen_dim, text_to_draw, v2m(gs->screen_dim.x - w,32), font_scale, true);

  sprintf(text_to_draw, "A: pressed:%d-down:%d-released:%d-up:%d", input_key_pressed(KEY_SCANCODE_A), input_key_down(KEY_SCANCODE_A), input_key_released(KEY_SCANCODE_A), input_key_up(KEY_SCANCODE_A));
  f32 wks = font_util_measure_text_width(&gs->font, text_to_draw, font_scale);
  font_util_debug_draw_text(&gs->font, gs->frame_arena, gs->screen_dim, text_to_draw, v2m(gs->screen_dim.x/2 - wks/2,64), 0.5, true);

  //sprintf(text_to_draw, "MMB: pressed:%d-down:%d-released:%d-up:%d", input_mkey_pressed(INPUT_MOUSE_MMB), input_mkey_down(INPUT_MOUSE_MMB), input_mkey_released(INPUT_MOUSE_MMB), input_mkey_up(INPUT_MOUSE_MMB));
  sprintf(text_to_draw, "mousepos: %f %f", input_get_mouse_pos().x, input_get_mouse_pos().y);
  f32 wks2 = font_util_measure_text_width(&gs->font, text_to_draw, font_scale);
  font_util_debug_draw_text(&gs->font, gs->frame_arena, gs->screen_dim, text_to_draw, v2m(gs->screen_dim.x/2 - wks2/2,128), 0.5, true);

  float speedup = 3.0;
  f64 ts = platform_get_time()*speedup;
  R2D* rend = r2d_begin(gs->frame_arena, &(R2D_Cam){ .offset = v2m(gs->screen_dim.x/2.0, gs->screen_dim.y/2.0), .origin = v2m(5,5), .zoom = 30.0, .rot_deg = -ts,}, gs->screen_dim);

  r2d_push_quad(rend, (R2D_Quad) {
      .src_rect = (R2D_Rect){},
      .dst_rect = (R2D_Rect){0,0,10,10},
      .color = (R2D_Color){1,1,1,0.8},
      .tex = gs->red,
      .rot_deg = ts,
  });
  u32 xidx = (u32)ts % ATLAS_SPRITES_X;
  u32 yidx = (u32)ts / ATLAS_SPRITES_X;
  r2d_push_quad(rend, (R2D_Quad) {
      .src_rect = (R2D_Rect){xidx*8, yidx*8, 8,8},
      .dst_rect = (R2D_Rect){1,1,8,8},
      .color = (R2D_Color){1,1,1,1.0},
      .tex = gs->atlas,
      .rot_deg = ts,
  });
  r2d_end(rend);

  // Effect Testing
  Effect_Data fill_data = {
    .screen_dim = gs->screen_dim,
    .time_sec = platform_get_time(),
    .framerate = 1.0f/dt,
    .param0 = v4m(0.1,0.1,0.9,0.1),
    .param1 = v4m(1,1,1,1),
  };
  effect_render(&gs->fill_effect, &fill_data);

  gui_frame_begin(gs->screen_dim, dt);


  Gui_Box *right = gui_box_lookup_from_key(0, gui_key_from_str("panel_c2"));
  assert(!gui_box_is_nil(right));
  gui_push_parent(right);

  // Button1
	gui_set_next_pref_size(GUI_AXIS_X, (Gui_Size){.kind = GUI_SIZE_KIND_PIXELS, 120, 1.0});
  gui_set_next_bg_color(v4m(0.4,0.4,0.4,1));
  //if (gui_button("button1").flags & GUI_SIGNAL_FLAG_LMB_PRESSED) {
  if (gui_button("button1").flags & GUI_SIGNAL_FLAG_MOUSE_HOVER) {
    printf("hover over button1\n");
  }
  // Spacer 
  //gui_spacer((Gui_Size){.kind = GUI_SIZE_KIND_PIXELS, 300, 0.0});
  // Button2 
  gui_set_next_pref_size(GUI_AXIS_X, (Gui_Size){.kind = GUI_SIZE_KIND_PIXELS, 120, 1.0});
  gui_set_next_bg_color(v4m(0.3,0.3,0.3,1));
  if (gui_button("button2").flags & GUI_SIGNAL_FLAG_LMB_PRESSED) {
    printf("heyo2\n");
  }
  gui_pop_parent();

  Gui_Box *left = gui_box_lookup_from_key(0, gui_key_from_str("panel_c1"));
  assert(!gui_box_is_nil(left));
  gui_push_parent(left);
  for (u32 i = 0; i < 10; i+=1) {
    gui_set_next_pref_size(GUI_AXIS_X, (Gui_Size){.kind = GUI_SIZE_KIND_PARENT_PCT, 1.0, 1.0});
    gui_set_next_pref_size(GUI_AXIS_Y, (Gui_Size){.kind = GUI_SIZE_KIND_PARENT_PCT, 1.0, 0.0});
    gui_set_next_bg_color(v4m(i * 0.1, 0.2, 0.4, 1));
    char name[64];
    sprintf(name, "button_%i", i);
    gui_button(name);
  }
  gui_pop_parent();


  gui_frame_end();

}

