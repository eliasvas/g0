#ifndef _REND2D_H__
#define _REND2D_H__

// TODO: Texture handling is atrocious, make it possible (also Ogl side) to assign c-style texture sampler arrays to a slot
// TODO: Look at the batch fragment shader todos.. BEWARE!!

#include "helper.h"
#include "math3d.h"
#include "arena.h"
#include "ogl.h"

#define REND_MAX_INSTANCES 512
#define REND_MAX_TEXTURES 4

typedef struct {
  f32 r,g,b,a;
} Rend_Color;

typedef struct {
  f32 x,y,w,h;
} Rend_Rect;


typedef struct {
  v4 src_rect;
  v4 dst_rect;
  v4 color;
  f32 rot_rad;
  int tex_slot;
} Batch_Vertex;

typedef struct {
  Rend_Rect src_rect, dst_rect;
  Rend_Color color;
  f32 rot_rad;

  // TODO: Maybe this isn't the best way to conduct business.. Ogl_Tex is just a view
  Ogl_Tex tex;
} Rend_Quad;

typedef struct Rend_Quad_Chunk_Node Rend_Quad_Chunk_Node;
struct Rend_Quad_Chunk_Node {
  Rend_Quad_Chunk_Node *next;
  Rend_Quad *arr;
  u64 cap;
  u64 count;
};

typedef struct Rend_Quad_Chunk_List Rend_Quad_Chunk_List;
struct Rend_Quad_Chunk_List {
  // This is a Singly Linked-List Queue,
  // so we can access from the front i.e iterate array style
  Rend_Quad_Chunk_Node *first;
  Rend_Quad_Chunk_Node *last;

  u64 node_count;
  u64 quad_count;
};

typedef struct Rend_Quad_Array Rend_Quad_Array;
struct Rend_Quad_Array {
  Rend_Quad *arr;
  u64 count;
};

typedef struct {
  Ogl_Tex *textures;
  u64 count;
  u64 cap;
} Rend_Tex_Array;

typedef struct {
  Rend_Quad_Chunk_List list;
  Rend_Tex_Array tex_array;
  Arena *arena;
} Rend2D;


Rend2D* rend2d_begin(Arena *arena, v2 screen_dim);
void rend2d_end(Rend2D *rend);
void rend2d_push_quad(Rend2D *rend, Rend_Quad q);

#endif
