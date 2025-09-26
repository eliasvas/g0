#ifndef _FONT_UTIL_H__
#define _FONT_UTIL_H__

#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>
#include "ogl.h"
#include "arena.h"
#include "r2d.h"

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
  font.glyph_count = font.last_codepoint - font.first_codepoint + 1; 

  u8 *font_bitmap = (u8*)arena_push_array(arena, u8, sizeof(u8)*atlas_width*atlas_height);

  stbtt_packedchar *packed_chars = arena_push_array(arena, stbtt_packedchar, font.glyph_count);
  stbtt_aligned_quad *aligned_quads = arena_push_array(arena, stbtt_aligned_quad, font.glyph_count);

  // Pack all the needed glyphs to the bitmap and get their metrics (packedchar / aligned_quad)
  stbtt_pack_context pctx = {};
  stbtt_PackBegin(&pctx, font_bitmap, atlas_width, atlas_height, 0, 1, nullptr);
  stbtt_PackFontRange(&pctx, default_font_data, 0, glyph_height_in_px, font.first_codepoint, font.glyph_count, packed_chars);
  stbtt_PackEnd(&pctx);

  for (u32 glyph_idx = 0; glyph_idx < font.glyph_count; ++glyph_idx) {
    f32 trash_x, trash_y;
    stbtt_GetPackedQuad(packed_chars, atlas_width, atlas_height, glyph_idx, &trash_x, &trash_y, &aligned_quads[glyph_idx], 1);
  }

  // Calculate our internal font metrics, which we will use in-engine for font rendering
  for (u32 glyph_idx = 0; glyph_idx < font.glyph_count; ++glyph_idx) {
    stbtt_packedchar pc = packed_chars[glyph_idx];
    stbtt_aligned_quad ac = aligned_quads[glyph_idx];
    font.glyphs[glyph_idx].r = (rect){
      .x = pc.x0,
      .y = pc.y0,
      .w = pc.x1 - pc.x0,
      .h = pc.y1 - pc.y0,
    };
    // NOTE: Not sure if this one is needed..
    font.glyphs[glyph_idx].tc = (rect){
      .x = ac.s0,
      .y = ac.t0,
      .w = ac.s1 - ac.s0,
      .h = ac.t1 - ac.t0,
    };
    font.glyphs[glyph_idx].off = v2m(pc.xoff, pc.yoff);
    font.glyphs[glyph_idx].xadvance = pc.xadvance;
  }

  // @HACK, This is because stbtt_Pack API is made to pack glyphs so the SPACE on has
  // no size, which means also no xadvance I think, for that reason we use the Font API to populate its xadvance..
  stbtt_fontinfo font_info;
  stbtt_InitFont(&font_info, default_font_data, stbtt_GetFontOffsetForIndex(default_font_data, 0));
  f32 scale = stbtt_ScaleForPixelHeight(&font_info, glyph_height_in_px);
  int advance, lsb;
  u32 space_glyph_idx = ' ' - font.first_codepoint;
  stbtt_GetCodepointHMetrics(&font_info, font.first_codepoint + space_glyph_idx, &advance, &lsb);
  font.glyphs[space_glyph_idx].xadvance = advance * scale;
 

  // Transform to OpenGL-style texture (mainly by convention, I like the upright view on renderdoc) + make the actual texture
  font_util_flip_bitmap(font_bitmap, atlas_width, atlas_height);
  font.atlas = ogl_tex_make(font_bitmap, atlas_width, atlas_height, OGL_TEX_FORMAT_R8U, (Ogl_Tex_Params){.wrap_s = OGL_TEX_WRAP_MODE_REPEAT, .wrap_t = OGL_TEX_WRAP_MODE_REPEAT});
  //t = ogl_tex_make((u8[]){255}, 1,1, OGL_TEX_FORMAT_R8U, (Ogl_Tex_Params){.wrap_t = OGL_TEX_WRAP_MODE_REPEAT, .wrap_s = OGL_TEX_WRAP_MODE_REPEAT});

  return font;
}

// TODO: WHY strlen, we dont like the cstdlib here!
static rect font_util_calc_text_rect(Font_Info *font_info, char *text, v2 baseline_pos, f32 scale) {
  u32 glyph_count = strlen(text);
  if (glyph_count == 0) return (rect){};

  Glyph_Info first_glyph = font_info->glyphs[text[0] - font_info->first_codepoint];
  rect r = (rect) {
    .x = first_glyph.off.x +baseline_pos.x,
    .y = first_glyph.off.y +baseline_pos.y,
    .w = first_glyph.r.w,
    .h = first_glyph.r.h,
  };

  for (u32 glyph_idx = 0; glyph_idx < glyph_count; ++glyph_idx) {
    Glyph_Info glyph = font_info->glyphs[text[glyph_idx] - font_info->first_codepoint];
    rect r1 = (rect) {
      .x = glyph.off.x + baseline_pos.x,
      .y = glyph.off.y + baseline_pos.y,
      .w = glyph.r.w,
      .h = glyph.r.h,
    };
    baseline_pos.x += glyph.xadvance;
    r = rect_calc_bounding_rect(r, r1);
  }

  return r;
}

static f32 font_util_measure_text_width(Font_Info *font_info, char *text, f32 scale) {
  return font_util_calc_text_rect(font_info, text, v2m(0,0), scale).w;
}

static f32 font_util_measure_text_height(Font_Info *font_info, char *text, f32 scale) {
  return font_util_calc_text_rect(font_info, text, v2m(0,0), scale).h;
}

static void font_util_debug_draw_text(Font_Info *font_info, Arena *arena, v2 screen_dim, char *text, v2 baseline_pos, f32 scale) {
  R2D* text_rend = r2d_begin(arena, &(R2D_Cam){ .offset = v2m(0, 0), .origin = v2m(0,0), .zoom = 1.0, .rot_deg = 0.0, }, screen_dim);

  rect tr = font_util_calc_text_rect(font_info, text, baseline_pos, scale);
  r2d_push_quad(text_rend, (R2D_Quad) {
      .dst_rect = *(R2D_Rect*)&tr,
      .color = (R2D_Color){1,0,1,1},
  });

  for (u32 i = 0; i < strlen(text); ++i) {
    u8 c = text[i];
    Glyph_Info metrics = font_info->glyphs[c - font_info->first_codepoint];
    r2d_push_quad(text_rend, (R2D_Quad) {
        .dst_rect = (R2D_Rect){baseline_pos.x+metrics.off.x*scale, baseline_pos.y+metrics.off.y*scale, metrics.r.w*scale, metrics.r.h*scale},
        .src_rect = (R2D_Rect){metrics.r.x, metrics.r.y, metrics.r.w, metrics.r.h},
        .color = (R2D_Color){0.9,0.9,0.3,1},
        .tex = font_info->atlas,
    });
    baseline_pos.x += metrics.xadvance*scale;
  }

  r2d_end(text_rend);
}


#endif
