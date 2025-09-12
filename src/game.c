#include "helper.h"
#include "math3d.h"
#include "ogl.h"

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
out vec4 out_color;
in vec3 f_color;
in vec2 f_tc;
layout (std140) uniform UboExample { float modifier; float pad0; float pad1; float pad2;};

uniform sampler2D tex;
void main() {
  out_color = vec4(f_color, 1.0f)/20.0 + modifier * texture(tex, f_tc);
}

)";

global_var Ogl_Render_Bundle rbundle = {};

void game_init(void) {
  ogl_init(); // To create the bullshit empty VAO opengl side, nothing else

  rbundle = (Ogl_Render_Bundle){
    .sp = ogl_shader_make(vs_source, fs_source),
    .vbos = {
      [0] = {
        //Ogl_Buf ogl_buf_make(Ogl_Buf_Kind kind, Ogl_Buf_Hint hint, void *data, uint32_t count, uint32_t bytes_per_elem) {
        .buffer = ogl_buf_make(OGL_BUF_KIND_VERTEX, OGL_BUF_HINT_STATIC, (f32[]) {
              -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f,    // top left
              -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
               0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
               0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
            }, 4, 8*sizeof(f32)),
        .vattribs = {
          [0] = { .location = 0, .type = OGL_DATA_TYPE_VEC3, .offset = 0 },
          [1] = { .location = 1, .type = OGL_DATA_TYPE_VEC3, .offset = 3*sizeof(f32) },
          [2] = { .location = 2, .type = OGL_DATA_TYPE_VEC2, .offset = 6*sizeof(f32) },
        },
      },
    },
    .ubos = {
      [0] = { .name = "UboExample", .buffer = ogl_buf_make(OGL_BUF_KIND_UNIFORM, OGL_BUF_HINT_DYNAMIC, (f32[]) { 0.9, 0,0,0 }, 1, sizeof(f32)*4), .start_offset = 0, .size = sizeof(float)*4 },
    },
    .dyn_state = (Ogl_Dyn_State){
      .viewport = {0,0,800,600},
    }
  };

  Platform_Image_Data image = platform_load_image_bytes_as_rgba("data/microgue.png");
  assert(image.width > 0); assert(image.height > 0);assert(image.data != nullptr);

  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data);
  glGenerateMipmap(GL_TEXTURE_2D);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);
}

// TODO: make a game.h -> make a Game_Event thingy with SLL -> pass to update
void game_update(float dt) { }

void game_render(void) {
  ogl_clear((Ogl_Color){0.2,0.2,0.25,1.0});
  ogl_render_bundle_draw(&rbundle, OGL_PRIM_TYPE_TRIANGLE_FAN, 4, 1);
}

