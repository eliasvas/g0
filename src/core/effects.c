#include "core/core_inc.h"

const char* fullscreen_vs = R"(#version 300 es
precision highp float;
out vec2 f_tc;
layout (std140) uniform EffectUBO {
  vec2 screen_dim;
  vec2 screen_offset;
  float time_sec;
  float framerate;
  vec2 pad0;
  vec4 param0;
  vec4 param1;
};
void main() {
    f_tc = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    gl_Position = vec4(f_tc * 2.0f + -1.0f, 0.0f, 1.0f);
  }
)";

const char* vortex_fs = R"(#version 300 es
precision highp float;
layout(location = 0) out vec4 out_color;
in vec2 f_tc;
layout (std140) uniform EffectUBO {
  vec2 screen_dim;
  vec2 screen_offset;
  float time_sec;
  float framerate;
  vec2 pad0;
  vec4 param0;
  vec4 param1;
};
#define PI 3.14159265359
//uniform sampler2D tex;
void main() {
  float transparency = param0.x;
  vec2 uv = (gl_FragCoord.xy - screen_offset) / screen_dim / 0.5 - 1.0;
  uv.x *= screen_dim.x / screen_dim.y;
  float f = 1.0 / length(uv);
  f += atan(uv.x, uv.y) / acos(0.);
  f -= time_sec;
  f = 1.0 - clamp(sin(f * PI * 2.0) * dot(uv, uv) * screen_dim.y / 15.0 + 0.5, 0.0, 1.0);
  f *= sin(length(uv) - .1);
  out_color = vec4(f,f,f,1.0) * transparency;
}

)";

const char* fill_fs = R"(#version 300 es
precision highp float;
layout(location = 0) out vec4 out_color;
in vec2 f_tc;
layout (std140) uniform EffectUBO {
  vec2 screen_dim;
  vec2 screen_offset;
  float time_sec;
  float framerate;
  vec2 pad0;
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

  // For example, we can attach rt's or textures or stuff depending on kind
  switch (e.kind) {
    case EFFECT_KIND_NONE:
      break;
    case EFFECT_KIND_BLUR_SOURCE:
      break;
    case EFFECT_KIND_PIXELATE_SOURCE:
      break;
    case EFFECT_KIND_VORTEX:
      e.bundle = (Ogl_Render_Bundle){
        .sp = ogl_shader_make(fullscreen_vs, vortex_fs),
        .ubos = { [0] = { .name = "EffectUBO", .buffer = ogl_buf_make(OGL_BUF_KIND_UNIFORM, OGL_BUF_HINT_DYNAMIC, nullptr, 1, sizeof(Effect_Data)), .start_offset = 0, .size = sizeof(m4) }, },
        .dyn_state = (Ogl_Dyn_State){ .flags = OGL_DYN_STATE_FLAG_BLEND, }
      };
      break;
    case EFFECT_KIND_FILL:
      e.bundle = (Ogl_Render_Bundle){
        .sp = ogl_shader_make(fullscreen_vs, fill_fs),
        .ubos = { [0] = { .name = "EffectUBO", .buffer = ogl_buf_make(OGL_BUF_KIND_UNIFORM, OGL_BUF_HINT_DYNAMIC, nullptr, 1, sizeof(Effect_Data)), .start_offset = 0, .size = sizeof(m4) }, },
        .dyn_state = (Ogl_Dyn_State){ .flags = OGL_DYN_STATE_FLAG_BLEND, }
      };
      break;
    case EFFECT_KIND_COUNT:
      break;
    default: break;
  }

  return e;
}

void effect_destroy(Effect *e) {
  if (e->kind != EFFECT_KIND_NONE) {
    ogl_render_bundle_destroy(&e->bundle);
    e->kind = EFFECT_KIND_NONE;
  }
}


void effect_render(Effect *effect, Effect_Data *data, v2 screen_dim, rect viewport) {
  effect->bundle.dyn_state.viewport = viewport;
  data->screen_dim = v2m(viewport.w, viewport.h);

  // Do we ACTUALLY need this?
  data->screen_offset = v2m(viewport.x, viewport.y);

  switch (effect->kind) {
    case EFFECT_KIND_NONE:
      return;
      break;
    case EFFECT_KIND_BLUR_SOURCE:
      break;
    case EFFECT_KIND_PIXELATE_SOURCE:
      break;
    case EFFECT_KIND_VORTEX:
      break;
    case EFFECT_KIND_FILL:
      // here param0 is the color
      break;
    case EFFECT_KIND_COUNT:
      break;
    default: break;
  }

  ogl_buf_update(&effect->bundle.ubos[0].buffer, 0, data, 1, sizeof(Effect_Data));
  ogl_render_bundle_draw(&effect->bundle, OGL_PRIM_TYPE_TRIANGLE, 3, 1);
}

