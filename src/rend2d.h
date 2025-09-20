#ifndef _REND2D_H__
#define _REND2D_H__

#include "helper.h"
#include "arena.h"
/*
 * Goal Today: Make a 2d Batch renderer, that will draw on an offscreen buffer
 * so we can have various effecs like blur when writing to the swapchain
*/

#define REND_MAX_INSTANCES 512

typedef struct {
  v4 src_rect;
  v4 dst_rect;
  v4 color;
  f32 rot_rad;
} Batch_Vertex;

const char* batch_vs = R"(#version 300 es
precision highp float;
layout(location=0) in vec4 src_rect;
layout(location=1) in vec4 dst_rect;
layout(location=2) in vec4 v_color;
layout(location=3) in float v_rot_rad;
//layout(location=4) in int v_tex_slot;

layout (std140) uniform BatchUbo { mat4 view_proj; };

vec2 vertices[4] = vec2[](
  vec2(-0.5,-0.5),
  vec2(+0.5,-0.5),
  vec2(-0.5,+0.5),
  vec2(+0.5,+0.5)
);

vec2 tex_coords[4] = vec2[](
  vec2(0.0,0.0),
  vec2(1.0,0.0),
  vec2(0.0,1.0),
  vec2(1.0,1.0)
);

out vec4 f_color;
out vec2 f_tc;
//flat out int v_tex_slot;

mat2 rotate2d(float angle) { return mat2(cos(angle), -sin(angle), sin(angle),  cos(angle)); }

void main() { 
  vec2 pos_offset = dst_rect.xy;
  vec2 dim = dst_rect.zw;

  vec2 hdim = vec2(0.5,0.5);

  vec2 pos = vertices[gl_VertexID]; // [-0.5, 0.5] range
  pos *= dim; // scale
  pos = rotate2d(v_rot_rad) * pos; // rotate

  pos += hdim * dim; // += hdim so that its centered on upper-left corner

  pos += pos_offset; // translate

	gl_Position = view_proj * vec4(pos, 0.0, 1.0);

  f_color = v_color;
  //f_tex_slot = v_tex_slot;
  f_tc = src_rect.xy + tex_coords[gl_VertexID] * src_rect.zw;
}
)";

const char* batch_fs = R"(#version 300 es
precision highp float;
layout(location = 0) out vec4 out_color;

in vec2 f_tc;
in vec4 f_color;
//flat in int f_tex_slot;

//uniform sampler2D u_textures[4];
uniform sampler2D tex;

void main() {
  //vec2 tc = f_tc / textureSize(u_textures[v_tex_slot], 0);
  //o_color = f_color * texture(u_textures[v_tex_slot], tc);

  ivec2 texture_size = textureSize(tex, 0);
  vec2 tc = f_tc / vec2(texture_size.x, texture_size.y);
  out_color = f_color * texture(tex, tc);
}

)";

typedef struct {
  f32 r,g,b,a;
} Rend_Color;

typedef struct {
  f32 x,y,w,h;
} Rend_Rect;

typedef struct {
  Rend_Rect src_rect, dst_rect;
  Rend_Color color;
  f32 rot_rad;
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
  Rend_Quad_Chunk_List list;
  Arena *arena;
} Rend2D;


// Maybe the Rend_Quad should be passed by pointer, it might be YUGE!
static void rend_quad_chunk_list_push(Arena *arena, Rend_Quad_Chunk_List* list, u64 cap, Rend_Quad quad) {
  Rend_Quad_Chunk_Node *node = list->first;
  if (node == nullptr || node->count >= node->cap) {
    node = arena_push_struct(arena, Rend_Quad_Chunk_Node);
    node->arr = arena_push_array(arena, Rend_Quad_Chunk_Node, cap);
    node->cap = cap;
    sll_queue_push(list->first, list->last, node);
    list->node_count+=1;
  }
  node->arr[node->count] = quad;
  node->count+=1;
  list->quad_count+=1;
}

static Rend_Quad_Array rend_quad_chunk_list_to_array(Arena *arena, Rend_Quad_Chunk_List *list) {
  Rend_Quad_Array qa = {};

  qa.count = list->quad_count;
  qa.arr = arena_push_array(arena, Rend_Quad, qa.count);
  u64 itr = 0;
  for (Rend_Quad_Chunk_Node *node = list->first; node != nullptr; node = node->next) {
    M_COPY(&qa.arr[itr], node->arr, sizeof(Rend_Quad)*node->count);
    itr += node->count;
    assert(itr <= qa.count);
  }
  assert(itr == qa.count);


  return qa;
}

static Ogl_Render_Bundle batch_bundle = {};

static Rend2D* rend2d_begin(Arena *arena, v2 screen_dim) {

  // INLINE m4 m4_look_at(v3 eye, v3 center, v3 f_up) {
  // INLINE m4 m4_ortho(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f) {
  if (batch_bundle.sp.impl_state == 0) {
    //m4 m = m4_mult(m4_look_at(v3m(0,0,-1), v3m(0,0,0), v3m(0,1,0)), m4_ortho(0,screen_dim.x,0,screen_dim.y,-1,1));
    //m4 m = m4_ortho(0,screen_dim.x,0,screen_dim.y,-1,1);
    //m4 m = m4_mult(m4_ortho(0,screen_dim.x,0,screen_dim.y,-1,1), m4_look_at(v3m(0,0,-1), v3m(0,0,0), v3m(0,1,0)));
    m4 m = m4d(1.0);
    batch_bundle = (Ogl_Render_Bundle){
      .sp = ogl_shader_make(batch_vs, batch_fs),
      .vbos = {
        [0] = {
          .buffer = ogl_buf_make(OGL_BUF_KIND_VERTEX, OGL_BUF_HINT_STATIC,nullptr, REND_MAX_INSTANCES, sizeof(Batch_Vertex)),
          .vattribs = {
            [0] = { .location = 0, .type = OGL_DATA_TYPE_VEC4,  .offset = offsetof(Batch_Vertex, src_rect), .stride = sizeof(Batch_Vertex), .instanced = true, },
            [1] = { .location = 1, .type = OGL_DATA_TYPE_VEC4,  .offset = offsetof(Batch_Vertex, dst_rect), .stride = sizeof(Batch_Vertex),.instanced = true, },
            [2] = { .location = 2, .type = OGL_DATA_TYPE_VEC4,  .offset = offsetof(Batch_Vertex, color),    .stride = sizeof(Batch_Vertex),.instanced = true, },
            [3] = { .location = 3, .type = OGL_DATA_TYPE_FLOAT, .offset = offsetof(Batch_Vertex, rot_rad),  .stride = sizeof(Batch_Vertex),.instanced = true, },
          },
        },
      },
      .ubos = {
        [0] = { .name = "BatchUbo", .buffer = ogl_buf_make(OGL_BUF_KIND_UNIFORM, OGL_BUF_HINT_DYNAMIC, (m4[]) { m }, 1, sizeof(m4)), .start_offset = 0, .size = sizeof(m4) },
      },
      .textures = {
        [0] = { .name = "tex", .tex = ogl_tex_make((u8[]){200,60,60,255}, 1,1, OGL_TEX_FORMAT_RGBA8U, (Ogl_Tex_Params){.wrap_s = OGL_TEX_WRAP_MODE_REPEAT}),},
      },
      //.rt = ogl_render_target_make(screen_dim.x, screen_dim.y, 2, OGL_TEX_FORMAT_RGBA8U, true),
      .dyn_state = (Ogl_Dyn_State){
        .viewport = {0,0,screen_dim.x,screen_dim.y},
      }
    };
  }
  batch_bundle.dyn_state.viewport = (Ogl_Rect){0,0,screen_dim.x,screen_dim.y};

  Rend2D *rend = arena_push_array(arena, Rend2D, 1);
  rend->arena = arena;

  return rend;
}

static void rend2d_flush(Rend2D *rend, Batch_Vertex *vertices, u64 count) {
  ogl_buf_update(&batch_bundle.vbos[0].buffer, 0, vertices, count, sizeof(Batch_Vertex));
  ogl_render_bundle_draw(&batch_bundle, OGL_PRIM_TYPE_TRIANGLE_STRIP, 4, count);
}
static void rend2d_end(Rend2D *rend) {
  Rend_Quad_Array quads = rend_quad_chunk_list_to_array(rend->arena, &rend->list);

  Batch_Vertex *batch_vertices = arena_push_array(rend->arena, Batch_Vertex,REND_MAX_INSTANCES);
  for (u64 quad_idx = 0; quad_idx < quads.count; ++quad_idx) {
    Rend_Quad *q = &quads.arr[quad_idx];
    printf("Trying to draw Quad %f %f %f %f\n", q->src_rect.x, q->src_rect.y, q->src_rect.w, q->src_rect.h);

    batch_vertices[quad_idx % REND_MAX_INSTANCES] = (Batch_Vertex){
      .src_rect = v4m(q->src_rect.x, q->src_rect.y, q->src_rect.w, q->src_rect.h),
      .dst_rect = v4m(q->dst_rect.x, q->dst_rect.y, q->dst_rect.w, q->dst_rect.h),
      .color = v4m(q->color.r, q->color.g, q->color.b, q->color.a),
      .rot_rad = q->rot_rad,
    };

    if ((quad_idx != 0 && (quad_idx % REND_MAX_INSTANCES == 0)) || (quad_idx+1 >= quads.count)) {
      rend2d_flush(rend, batch_vertices, quad_idx % REND_MAX_INSTANCES + 1);
    }

  }

}

static void rend2d_push_quad(Rend2D *rend, Rend_Quad q) {
  rend_quad_chunk_list_push(rend->arena, &rend->list, 256, q);
}

#endif
