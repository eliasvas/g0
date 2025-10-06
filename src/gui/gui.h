#ifndef GUI_H__
#define GUI_H__

#include "base/base_inc.h"
#include "core/core_inc.h"

//rect font_util_calc_text_rect(Font_Info *font_info, char *text, v2 baseline_pos, f32 scale);
//f32 font_util_measure_text_width(Font_Info *font_info, char *text, f32 scale);
//f32 font_util_measure_text_height(Font_Info *font_info, char *text, f32 scale);
//void font_util_debug_draw_text(Font_Info *font_info, Arena *arena, v2 screen_dim, char *text, v2 baseline_pos, f32 scale);

typedef struct {
  Arena *temp_arena;
  Font_Info *font;

  v2 screen_dim;
  float font_scale;

  u64 hot_id;
  u64 active_id;

} Gui_Context;

static Gui_Context g_gui_ctx;

static void gui_context_init(Arena *temp_arena, Font_Info *font) {
  g_gui_ctx.temp_arena = temp_arena;
  g_gui_ctx.font = font;
  g_gui_ctx.font_scale = 0.3;
}

void gui_begin(v2 screen_dim) {
  g_gui_ctx.screen_dim = screen_dim;
  g_gui_ctx.hot_id = 0;
}

void gui_end() { }

//static bool rect_isect_point(rect r, v2 p) {

static bool gui_button(u64 id, rect r, char *label) {
  bool res = false;

  rect label_rect = font_util_calc_text_rect(g_gui_ctx.font, label, v2m(0,0), g_gui_ctx.font_scale);
  rect fitted_rect = rect_try_fit_inside(label_rect, r);

  v2 top_left = v2m(fitted_rect.x, fitted_rect.y);
  v2 baseline = v2_sub(top_left, label_rect.p0);

  if (rect_isect_point(r, input_get_mouse_pos())) {
    g_gui_ctx.hot_id = id;
  }
  if (g_gui_ctx.hot_id == id && input_mkey_pressed(INPUT_MOUSE_LMB)) {
    g_gui_ctx.active_id = id;
  } 
  if (g_gui_ctx.active_id == id && input_mkey_released(INPUT_MOUSE_LMB)) {
    g_gui_ctx.active_id = 0;
    if (g_gui_ctx.hot_id == id) {
      res = true;
    }
  }


  R2D_Color c = (g_gui_ctx.active_id == id) ? (R2D_Color){0.6,0.4,0.4,1} : (g_gui_ctx.hot_id == id) ? (R2D_Color){0.5,0.4,0.4,1} : (R2D_Color){0.4,0.4,0.4,1};

  // TODO: make a 'default' camera initializer or macro or something, this is too long
  R2D* text_rend = r2d_begin(g_gui_ctx.temp_arena, &(R2D_Cam){ .offset = v2m(0,0), .origin = v2m(0,0), .zoom = 1.0, .rot_deg = 0.0, }, g_gui_ctx.screen_dim);
  r2d_push_quad(text_rend, (R2D_Quad) {
      .dst_rect = *(R2D_Rect*)&r,
      .color = c, // TODO: styles
  });
  r2d_end(text_rend);

  font_util_debug_draw_text(g_gui_ctx.font, g_gui_ctx.temp_arena, g_gui_ctx.screen_dim, label, baseline, g_gui_ctx.font_scale, false);

  return res;
}

#endif
