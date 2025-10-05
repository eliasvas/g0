#ifndef EFFECTS_H__
#define EFFECTS_H__
#include "base/base_inc.h"

typedef enum {
  EFFECT_KIND_BLUR_SOURCE,
  EFFECT_KIND_PIXELATE_SOURCE,
  EFFECT_KIND_VORTEX,
  EFFECT_KIND_FILL,
  EFFECT_KIND_COUNT,
} Effect_Kind;


typedef struct {
  Ogl_Render_Bundle bundle;
  Effect_Kind kind;
} Effect;

// https://wikis.khronos.org/opengl/Interface_Block_(GLSL) - std140 layout
typedef struct {
  v2 screen_dim;
  f32 time_sec;
  f32 framerate;
  v4 param0;
  v4 param1;
} Effect_Data;

Effect effect_make(Effect_Kind kind);
void effect_render(Effect *effect, Effect_Data *data);

#endif
