#include "game.h"
#include "ogl.h"
#include "rend2d.h"

// Comment this out if not protoyping
#include <GL/glew.h>

/*
const u8 font_data[] = {
#embed "../data/Ubuntu_Mono/UbuntuMono-Regular.ttf"
};
*/

static Ogl_Tex test_tex1;
static Ogl_Tex test_tex2;

void game_init(Game_State *gs) {
  ogl_init(); // To create the bullshit empty VAO opengl side, nothing else

  test_tex1 = ogl_tex_make((u8[]){0,60,160,255}, 1,1, OGL_TEX_FORMAT_RGBA8U, (Ogl_Tex_Params){.wrap_s = OGL_TEX_WRAP_MODE_REPEAT});
  test_tex2 = ogl_tex_make((u8[]){200,60,60,255}, 1,1, OGL_TEX_FORMAT_RGBA8U, (Ogl_Tex_Params){.wrap_s = OGL_TEX_WRAP_MODE_REPEAT});

#if 0
  // Arena test
  Arena* test_arena = arena_make(MB(256));
  u32 elem_count = 500;

  f32 *my_floats = arena_push(test_arena, sizeof(float)*elem_count);
  assert(my_floats);
  for (u32 i = 0; i < elem_count; ++i) { my_floats[i] = (f32)i; }

  f32 *my_floats2 = arena_push(test_arena, sizeof(float)*elem_count*2);
  assert(my_floats2);
  for (u32 i = 0; i < elem_count*2; ++i) { my_floats2[i] = (f32)i; }

  Ogl_Render_Bundle *my_unused_render_bundle = arena_push_struct(test_arena, Ogl_Render_Bundle);
  assert(my_unused_render_bundle);

  arena_clear(test_arena);
  arena_destroy(test_arena);
#endif 

}

// TODO: make a game.h -> make a Game_Event thingy with SLL -> pass to update
void game_update(Game_State *gs, float dt) { }

void game_render(Game_State *gs) {
  ogl_clear((Ogl_Color){0.2,0.2,0.25,1.0});

  Rend2D* rend = rend2d_begin(gs->frame_arena, gs->screen_dim);
  assert(rend);
  rend2d_push_quad(rend, (Rend_Quad) {
      .src_rect = (Rend_Rect){0,0,100,100},
      //.dst_rect = (Rend_Rect){0,0,0.1,0.2},
      .dst_rect = (Rend_Rect){100,0,100,120},
      .color = (Rend_Color){1,1,1,1},
      .tex = test_tex1,
  });
  rend2d_push_quad(rend, (Rend_Quad) {
      .src_rect = (Rend_Rect){0,0,100,100},
      //.dst_rect = (Rend_Rect){0.3,0,0.1,0.2},
      .dst_rect = (Rend_Rect){0,0,100,300},
      .color = (Rend_Color){0.6,1,1,1},
      .tex = test_tex2,
  });
  //rend2d_push_quad(rend, (Rend_Quad) {});
  //rend2d_push_quad(rend, (Rend_Quad) {});
  rend2d_end(rend);

}


