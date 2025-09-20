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

// Forward Declarations, I don't like this.. @FIXME
typedef struct { u8 *data; u64 width; u64 height; } Platform_Image_Data;
Platform_Image_Data platform_load_image_bytes_as_rgba(const char *filepath);
f64 platform_get_time();

const char* off_vs_source = R"(#version 300 es
precision highp float;
out vec2 f_tc;
void main() { 
    f_tc = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    gl_Position = vec4(f_tc * 2.0f + -1.0f, 0.0f, 1.0f);
  }
)";

const char* off_fs_source = R"(#version 300 es
precision highp float;
layout(location = 0) out vec4 out_color;
in vec2 f_tc;
uniform sampler2D tex;
void main() {
  //out_color = texture(tex, f_tc);
  out_color = vec4(0, texture(tex, f_tc).gb,1);
}

)";


const char* vs_source = R"(#version 300 es
precision highp float;
layout (location = 0) in vec3 v_pos;
layout (location = 1) in vec3 v_col;
layout (location = 2) in vec2 v_tc;
out vec3 f_color;
out vec2 f_tc;
void main() { gl_Position = vec4(v_pos, 1.0f); f_color = v_col; f_tc = v_tc; }
)";

const char* fs_source = R"(#version 300 es
precision highp float;
layout(location = 0) out vec4 out_color;
layout(location = 1) out vec4 out_color2;
in vec3 f_color;
in vec2 f_tc;
layout (std140) uniform UboExample { vec4 ubo_tc; };

uniform sampler2D tex;
uniform sampler2D tex2;
void main() {
  vec2 real_tc = ubo_tc.xy + f_tc * ubo_tc.zw;
  out_color = texture(tex, real_tc);
  out_color2 = vec4(f_color, 1.0) * texture(tex, f_tc);
}

)";

//global_var Ogl_Render_Bundle rbundle = {};

//global_var Ogl_Render_Bundle full_quad_bundle = {};

void game_init(Game_State *gs) {
  ogl_init(); // To create the bullshit empty VAO opengl side, nothing else

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
void game_update(Game_State *gs, float dt) {

  /*
#define SIZE_X (1.0/16.0)
#define SIZE_Y (1.0/10.0)

  f64 ts = platform_get_time()*2;
  u32 xidx = (u32)ts % 16;
  u32 yidx = (u32)ts / 10;

  v4 tc = v4m(SIZE_X*xidx, SIZE_Y*yidx, SIZE_X, SIZE_Y);
  ogl_buf_update(&rbundle.ubos[0].buffer, 0, &tc, 1, sizeof(v4));


  // TODO: Maybe this should be a function? or no, because semantic compression
  v2 screen_dim = gs->screen_dim;
  if ((s32)screen_dim.x != (s32)rbundle.rt.width || (s32)screen_dim.y != (s32)rbundle.rt.height) {
    ogl_render_target_deinit(&rbundle.rt);
    rbundle.rt = ogl_render_target_make(gs->screen_dim.x, gs->screen_dim.y, 2, OGL_TEX_FORMAT_RGBA8U, true);
    assert(gs->screen_dim.x);
    assert(gs->screen_dim.y);
  }
  */

}

void game_render(Game_State *gs) {
  ogl_clear((Ogl_Color){0.2,0.2,0.25,1.0});

  Rend2D* rend = rend2d_begin(gs->frame_arena, gs->screen_dim);
  assert(rend);
  rend2d_push_quad(rend, (Rend_Quad) {
      .src_rect = (Rend_Rect){0,0,100,100},
      .dst_rect = (Rend_Rect){0,0,0.1,0.2},
      .color = (Rend_Color){1,1,1,1},
  });

  //rend2d_push_quad(rend, (Rend_Quad) {});
  //rend2d_push_quad(rend, (Rend_Quad) {});
  rend2d_end(rend);

}


