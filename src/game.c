#include "helper.h"
#include <GL/glew.h>
#include "ogl.h"

/*
const u8 font_data[] = {
#embed "../data/Ubuntu_Mono/UbuntuMono-Regular.ttf"
};
*/

// For some reason AT LEAST the vetex shader compilation fails on Emscripten
const char* vs_source = R"(#version 300 es
precision highp float;
layout (location = 0) in vec3 v_pos;
layout (location = 1) in vec3 v_col;
out vec3 f_color;
void main() { gl_Position = vec4(v_pos, 1.0f); f_color = v_col; }
)";

const char* fs_source = R"(#version 300 es
precision highp float;
out vec4 out_color;
in vec3 f_color;
void main() { out_color = vec4(f_color, 1.0f); }
)";

global_var Ogl_Render_Bundle rbundle = {0};

void game_init(void) {
  ogl_init(); // To create the bullshit empty VAO opengl side, nothing else
  
  f32 vertices[] = {
    -0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,
     0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,
     0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f
  };

  rbundle = (Ogl_Render_Bundle){
    .sp = ogl_shader_make(vs_source, fs_source),
    .vertex_buffer = ogl_buf_make(OGL_BUF_KIND_VERTEX, vertices, 3, sizeof(v3) * 2),
    .vattribs = {
      [0] = { .location = 0, .type = OGL_DATA_TYPE_VEC3, .offset = 0, .avail = true },
      [1] = { .location = 1, .type = OGL_DATA_TYPE_VEC3, .offset = sizeof(v3), .avail = true },
    },
    .dyn_state = (Ogl_Dyn_State){
      .viewport = v4m(0,0,800,600)
    }
  };
}

// TODO: make a game.h -> make a Game_Event thingy with SLL -> pass to update
void game_update(float dt) { }

void game_render(void) {
  ogl_clear(v4m(0.2,0.2,0.25,1.0));
  ogl_render_bundle_draw(&rbundle, OGL_PRIM_TYPE_TRIANGLE, 3, 1);
}

