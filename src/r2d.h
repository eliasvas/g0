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
} R2D_Color;

typedef struct {
  f32 x,y,w,h;
} R2D_Rect;


typedef struct {
  v4 src_rect;
  v4 dst_rect;
  v4 color;
  f32 rot_rad;
  int tex_slot;
} Batch_Vertex;

typedef struct {
  R2D_Rect src_rect, dst_rect;
  R2D_Color color;
  f32 rot_rad;

  // TODO: Maybe this isn't the best way to conduct business.. Ogl_Tex is just a view
  Ogl_Tex tex;
} R2D_Quad;

typedef struct R2D_Quad_Chunk_Node R2D_Quad_Chunk_Node;
struct R2D_Quad_Chunk_Node {
  R2D_Quad_Chunk_Node *next;
  R2D_Quad *arr;
  u64 cap;
  u64 count;
};

typedef struct R2D_Quad_Chunk_List R2D_Quad_Chunk_List;
struct R2D_Quad_Chunk_List {
  // This is a Singly Linked-List Queue,
  // so we can access from the front i.e iterate array style
  R2D_Quad_Chunk_Node *first;
  R2D_Quad_Chunk_Node *last;

  u64 node_count;
  u64 quad_count;
};

typedef struct R2D_Quad_Array R2D_Quad_Array;
struct R2D_Quad_Array {
  R2D_Quad *arr;
  u64 count;
};

typedef struct {
  Ogl_Tex *textures;
  u64 count;
  u64 cap;
} R2D_Tex_Array;

typedef struct {
  R2D_Quad_Chunk_List list;
  R2D_Tex_Array tex_array;
  Arena *arena;
} R2D;

// TODO: make the trick with the macro for scale initialization
typedef struct {
  v2 offset;
  v2 origin;
  float zoom;
  // TODO: make this rad?
  float rot_deg;
} R2D_Cam;

R2D* r2d_begin(Arena *arena, R2D_Cam *cam, v2 screen_dim);
void r2d_end(R2D *rend);
void r2d_push_quad(R2D *rend, R2D_Quad q);

#endif
