#ifndef _FONT_UTIL_H__
#define _FONT_UTIL_H__
#include "helper.h"
#include "arena.h"
#include "ogl.h"
#include "math3d.h"

// TODO: LOD stuff and our own lookup data structure (Glyph_Cache?)
// TODO: SDF font support

typedef struct {
  rect r;
  rect tc;
  v2 off;
  f32 xadvance;
}Glyph_Info;

// TODO: Maybe we can use a stack allocator for the permanent arena, so the Font_Info's glyphs array can be allocated there (in the end)
typedef struct {
  Glyph_Info glyphs[200];
  u32 first_codepoint;
  u32 last_codepoint;
  u32 glyph_count;

  Ogl_Tex atlas;
}Font_Info;

void font_util_flip_bitmap(u8 *bitmap, u32 width, u32 height);
Font_Info font_util_load_default_atlas(Arena *arena, u32 glyph_height_in_px, u32 atlas_width, u32 atlas_height);

rect font_util_calc_text_rect(Font_Info *font_info, char *text, v2 baseline_pos, f32 scale);
f32 font_util_measure_text_width(Font_Info *font_info, char *text, f32 scale);
f32 font_util_measure_text_height(Font_Info *font_info, char *text, f32 scale);

void font_util_debug_draw_text(Font_Info *font_info, Arena *arena, v2 screen_dim, char *text, v2 baseline_pos, f32 scale);

#endif
