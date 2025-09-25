#ifndef _FONT_UTIL_H__
#define _FONT_UTIL_H__

#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>
#include "ogl.h"
#include "arena.h"

// TODO: LOD stuff and our own lookup data structure (Glyph_Cache?)
// TODO: SDF font support
// TODO: Load GL_RED textures, I think there is no need for RGBA


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

static const u8 default_font_data[] = {
#embed "../data/ProggyClean.ttf"
};

static void font_util_flip_bitmap(u8 *bitmap, u32 width, u32 height) {
  for (u32 y = 0; y < height/2; ++y) {
    for (u32 x = 0; x < width; ++x) {
      u8 temp = bitmap[x + y * width];
      bitmap[x+y*width] = bitmap[x+(height-y)*width];
      bitmap[x+(height-y)*width] = temp;
    }
  }
}

static Font_Info font_util_load_default_atlas(Arena *arena, u32 glyph_height_in_px, u32 atlas_width, u32 atlas_height) {
  Font_Info font = {};

  font.first_codepoint = 32; // ' ' 
  font.last_codepoint = 127; // '~'
  font.glyph_count = font.last_codepoint - font.first_codepoint; 

  u8 *font_bitmap = (u8*)arena_push_array(arena, u8, sizeof(u8)*atlas_width*atlas_height);

  stbtt_packedchar *packed_chars = arena_push_array(arena, stbtt_packedchar, font.glyph_count);
  stbtt_aligned_quad *aligned_quads = arena_push_array(arena, stbtt_aligned_quad, font.glyph_count);

  stbtt_pack_context pctx = {};
  stbtt_PackBegin(&pctx, font_bitmap, atlas_width, atlas_height, 0, 1, nullptr);
  stbtt_PackFontRange(&pctx, default_font_data, 0, glyph_height_in_px, font.first_codepoint, font.glyph_count, packed_chars);
  stbtt_PackEnd(&pctx);


  // Transform to OpenGL-style texture (mainly by convention, I like the upright view on renderdoc)
  font_util_flip_bitmap(font_bitmap, atlas_width, atlas_height);

  for (u32 glyph_idx = 0; glyph_idx < font.glyph_count; ++glyph_idx) {
    f32 trash_x, trash_y;
    stbtt_GetPackedQuad(packed_chars, atlas_width, atlas_height, glyph_idx, &trash_x, &trash_y, &aligned_quads[glyph_idx], 1);
  }

  for (u32 glyph_idx = 0; glyph_idx < font.glyph_count; ++glyph_idx) {
    stbtt_packedchar pc = packed_chars[glyph_idx];
    stbtt_aligned_quad ac = aligned_quads[glyph_idx];
    font.glyphs[glyph_idx].r = (rect){
      .x = pc.x0,
      .y = pc.y0,
      .w = pc.x1 - pc.x0,
      .h = pc.y1 - pc.y0,
    };
    font.glyphs[glyph_idx].tc = (rect){
      .x = ac.s0,
      .y = ac.t0,
      .w = ac.s1 - ac.s0,
      .h = ac.t1 - ac.t0,
    };
    font.glyphs[glyph_idx].off = v2m(pc.xoff, pc.yoff);
    font.glyphs[glyph_idx].xadvance = pc.xadvance;
  }

  font.atlas = ogl_tex_make(font_bitmap, atlas_width, atlas_height, OGL_TEX_FORMAT_R8U, (Ogl_Tex_Params){.wrap_s = OGL_TEX_WRAP_MODE_REPEAT, .wrap_t = OGL_TEX_WRAP_MODE_REPEAT});
  //t = ogl_tex_make((u8[]){255}, 1,1, OGL_TEX_FORMAT_R8U, (Ogl_Tex_Params){.wrap_t = OGL_TEX_WRAP_MODE_REPEAT, .wrap_s = OGL_TEX_WRAP_MODE_REPEAT});

  return font;
}

#endif
