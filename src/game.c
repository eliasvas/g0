#include "game.h"
#include "ogl.h"
#include "font_util.h"
#include "r2d.h"

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

  char text_to_draw[64];
  f32 fps = 1.0/dt;
  f32 font_scale = 0.5;
  sprintf(text_to_draw, "fps: %.2f", fps);
  f32 w = font_util_measure_text_width(&gs->font, text_to_draw, font_scale);
  font_util_debug_draw_text(&gs->font, gs->frame_arena, gs->screen_dim, text_to_draw, v2m(gs->screen_dim.x - w,32), font_scale);

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

  // Testing
  effect_render(&gs->fill_effect, gs->screen_dim, 1.0/dt, platform_get_time());
}

