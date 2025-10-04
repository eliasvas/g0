#include "effects.h"
#include "ogl.h"
#include "math3d.h"

const char* fs_vs = R"(#version 300 es
precision highp float;
out vec2 f_tc;
layout (std140) uniform EffectUBO {
  vec2 screen_dim;
  float time_sec;
  float framerate;
  vec4 param0;
  vec4 param1;
};
void main() {
    f_tc = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    gl_Position = vec4(f_tc * 2.0f + -1.0f, 0.0f, 1.0f);
  }
)";

const char* fs_fs = R"(#version 300 es
precision highp float;
layout(location = 0) out vec4 out_color;
in vec2 f_tc;
layout (std140) uniform EffectUBO {
  vec2 screen_dim;
  float time_sec;
  float framerate;
  vec4 param0;
  vec4 param1;
};
//uniform sampler2D tex;
void main() {
  out_color = param0;
}

)";

// Initialize the Render_Bundle for rendering effects..
// TODO: Should every effect have its own bundle? .. IDK
Effect effect_make(Effect_Kind kind) {
  Effect e = {};
  e.kind = kind;

  e.bundle = (Ogl_Render_Bundle){
    .sp = ogl_shader_make(fs_vs, fs_fs),
    .ubos = {
      [0] = { .name = "EffectUBO", .buffer = ogl_buf_make(OGL_BUF_KIND_UNIFORM, OGL_BUF_HINT_DYNAMIC, nullptr, 1, sizeof(Effect_Data)), .start_offset = 0, .size = sizeof(m4) },
    },
    //.rt = ogl_render_target_make(screen_dim.x, screen_dim.y, 2, OGL_TEX_FORMAT_RGBA8U, true),
    .dyn_state = (Ogl_Dyn_State){
      //.viewport = {0,0,screen_dim.x,screen_dim.y},
      .flags = OGL_DYN_STATE_FLAG_BLEND,
    }
  };

  // For example, we can attach rt's or textures or stuff depending on kind
  switch (e.kind) {
    case EFFECT_KIND_BLUR_SOURCE:
      break;
    case EFFECT_KIND_PIXELATE_SOURCE:
      break;
    case EFFECT_KIND_VORTEX:
      break;
    case EFFECT_KIND_FILL:
      break;
    case EFFECT_KIND_COUNT:
      break;
    default: break;
  }

  return e;
}

void effect_render(Effect *effect, v2 screen_dim, f32 time_sec, f32 fps) {
  effect->bundle.dyn_state.viewport = (Ogl_Rect){0,0,screen_dim.x,screen_dim.y};
  Effect_Data data = {
    .screen_dim = screen_dim,
    .time_sec = time_sec,
    .framerate = fps,
    .param0 = v4m(1,1,1,1),
    .param1 = v4m(1,1,1,1),
  };

  switch (effect->kind) {
    case EFFECT_KIND_BLUR_SOURCE:
      break;
    case EFFECT_KIND_PIXELATE_SOURCE:
      break;
    case EFFECT_KIND_VORTEX:
      break;
    case EFFECT_KIND_FILL:
      data.param0 = v4m(0,0.9,1,0.1);
      break;
    case EFFECT_KIND_COUNT:
      break;
    default: break;
  }

  ogl_buf_update(&effect->bundle.ubos[0].buffer, 0, &data, 1, sizeof(Effect_Data));

  ogl_render_bundle_draw(&effect->bundle, OGL_PRIM_TYPE_TRIANGLE, 3, 1);
}

