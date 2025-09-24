#include "r2d.h"
#include "ogl.h"
#include "math3d.h"

// TODO: WE SHOULD ENABLE multisampling ogl side! it should be a dynamic state right?

// HMMMMMMM
static Ogl_Render_Bundle batch_bundle = {};
static Ogl_Tex white_tex = {};

/////////////////////
// SHADERS
/////////////////////

const char* batch_vs = R"(#version 300 es
precision highp float;
layout(location=0) in vec4 src_rect;
layout(location=1) in vec4 dst_rect;
layout(location=2) in vec4 v_color;
layout(location=3) in float v_rot_rad;
layout(location=4) in int v_tex_slot;

layout (std140) uniform BatchUbo { mat4 view_proj; };

vec2 vertices[4] = vec2[](
  vec2(-0.5,+0.5),
  vec2(-0.5,-0.5),
  vec2(+0.5,-0.5),
  vec2(+0.5,+0.5)
);

vec2 tex_coords[4] = vec2[](
  vec2(0.0,0.0),
  vec2(0.0,1.0),
  vec2(1.0,1.0),
  vec2(1.0,0.0)
);

out vec4 f_color;
out vec2 f_tc;
flat out int f_tex_slot;

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
  f_tex_slot = v_tex_slot;
  f_tc = src_rect.xy*vec2(1,-1) + tex_coords[gl_VertexID] * src_rect.zw - vec2(0, src_rect.w);
}
)";

const char* batch_fs = R"(#version 300 es
precision highp float;
layout(location = 0) out vec4 out_color;

in vec2 f_tc;
in vec4 f_color;
flat in int f_tex_slot;

uniform sampler2D u_textures[4];

void main() {
    ivec2 texture_size;
    vec2 tc;
    // FIXME: I HATE this, GLES30 doesn't support c-style sampler2D array indexing so we have to hack..
    if (f_tex_slot == 0){
      texture_size = textureSize(u_textures[0], 0);
      tc = f_tc / vec2(texture_size.x, texture_size.y);
      out_color = f_color * texture(u_textures[0], tc);
    } else if (f_tex_slot == 1){
      texture_size = textureSize(u_textures[1], 0);
      tc = f_tc / vec2(texture_size.x, texture_size.y);
      out_color = f_color * texture(u_textures[1], tc);
    } else if (f_tex_slot == 2){
      texture_size = textureSize(u_textures[0], 0);
      tc = f_tc / vec2(texture_size.x, texture_size.y);
      out_color = f_color * texture(u_textures[1], tc);
    } else if (f_tex_slot == 3){
      texture_size = textureSize(u_textures[0], 0);
      tc = f_tc / vec2(texture_size.x, texture_size.y);
      out_color = f_color * texture(u_textures[1], tc);
    }
}

)";

/////////////////////
// Actual Implementation
/////////////////////

static m4 r2d_cam_make_view_mat(R2D_Cam *cam) {
  //m4 rot = m4d(1.0); // FIXME: implement rotations!
  m4 rot = mat4_rotate(cam->rot_deg, v3m(0,0,1));
  return m4_mult(m4_translate(v3m(cam->offset.x, cam->offset.y, 0)),m4_mult(rot,m4_mult(m4_scale(v3m(cam->zoom, cam->zoom,0)), m4_translate(v3m(-cam->origin.x, -cam->origin.y,0)))));
}

static s64 rend_tex_array_try_add(R2D_Tex_Array *tarray, Ogl_Tex tex) {
  for (u64 tex_idx = 0; tex_idx < tarray->count; ++tex_idx) {
    if (tarray->textures[tex_idx].impl_state == tex.impl_state) return tex_idx;
  }
  // If not found try to push_back
  if (tarray->count < tarray->cap){
    tarray->textures[tarray->count] = tex;
    return tarray->count++;
  }
  return -1;
}
static void rend_tex_array_clear(R2D_Tex_Array *tarray) {
  M_ZERO(tarray->textures, sizeof(Ogl_Tex) * tarray->cap);
  tarray->count = 0;
  tarray->cap = 0;
}

// Maybe the R2D_Quad should be passed by pointer, it might be YUGE!
static void rend_quad_chunk_list_push(Arena *arena, R2D_Quad_Chunk_List* list, u64 cap, R2D_Quad quad) {
  R2D_Quad_Chunk_Node *node = list->first;
  if (node == nullptr || node->count >= node->cap) {
    node = arena_push_struct(arena, R2D_Quad_Chunk_Node);
    node->arr = arena_push_array(arena, R2D_Quad_Chunk_Node, cap);
    node->cap = cap;
    sll_queue_push(list->first, list->last, node);
    list->node_count+=1;
  }
  node->arr[node->count] = quad;
  node->count+=1;
  list->quad_count+=1;
}

static R2D_Quad_Array rend_quad_chunk_list_to_array(Arena *arena, R2D_Quad_Chunk_List *list) {
  R2D_Quad_Array qa = {};

  qa.count = list->quad_count;
  qa.arr = arena_push_array(arena, R2D_Quad, qa.count);
  u64 itr = 0;
  for (R2D_Quad_Chunk_Node *node = list->first; node != nullptr; node = node->next) {
    M_COPY(&qa.arr[itr], node->arr, sizeof(R2D_Quad)*node->count);
    itr += node->count;
    assert(itr <= qa.count);
  }
  assert(itr == qa.count);


  return qa;
}

static void r2d_flush(R2D *rend, Batch_Vertex *vertices, u64 count) {
  // We set the correct texture for each slot, if not found, we just assign a dummy white texture for debug purposes
  char name_buf[REND_MAX_TEXTURES][64] = {};
  for (u64 tex_idx = 0; tex_idx < rend->tex_array.cap; ++tex_idx) {
    // TODO: This is too costly even for temporary storage, fix by making a static array of [MAX_TEXTURES][64]
    sprintf(name_buf[tex_idx], "u_textures[%lu]", tex_idx);
    if (tex_idx < rend->tex_array.count) {
      batch_bundle.textures[tex_idx] = (Ogl_Tex_Slot){ .name = name_buf[tex_idx], .tex = rend->tex_array.textures[tex_idx],};
    } else {
      batch_bundle.textures[tex_idx] = (Ogl_Tex_Slot){ .name = name_buf[tex_idx], .tex = white_tex,};
    }
  }
  ogl_buf_update(&batch_bundle.vbos[0].buffer, 0, vertices, count, sizeof(Batch_Vertex));
  ogl_render_bundle_draw(&batch_bundle, OGL_PRIM_TYPE_TRIANGLE_FAN, 4, count);
}

R2D* r2d_begin(Arena *arena, R2D_Cam *cam, v2 screen_dim) {
  //m4 m = m4_ortho(0,screen_dim.x,0,screen_dim.y,-1,1);
  m4 proj = m4_ortho(0,screen_dim.x,screen_dim.y,0,-1,1);
  m4 view = r2d_cam_make_view_mat(cam);
  m4 m = m4_mult(proj, view);

  if (batch_bundle.sp.impl_state == 0) {
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
            [4] = { .location = 4, .type = OGL_DATA_TYPE_INT, .offset = offsetof(Batch_Vertex, tex_slot),  .stride = sizeof(Batch_Vertex),.instanced = true, },
          },
        },
      },
      .ubos = {
        [0] = { .name = "BatchUbo", .buffer = ogl_buf_make(OGL_BUF_KIND_UNIFORM, OGL_BUF_HINT_DYNAMIC, (m4[]) { m }, 1, sizeof(m4)), .start_offset = 0, .size = sizeof(m4) },
      },
      //.rt = ogl_render_target_make(screen_dim.x, screen_dim.y, 2, OGL_TEX_FORMAT_RGBA8U, true),
      .dyn_state = (Ogl_Dyn_State){
        .viewport = {0,0,screen_dim.x,screen_dim.y},
        .flags = OGL_DYN_STATE_FLAG_BLEND,
      }
    };
    white_tex = ogl_tex_make((u8[]){255,255,255,255}, 1,1, OGL_TEX_FORMAT_RGBA8U, (Ogl_Tex_Params){.wrap_s = OGL_TEX_WRAP_MODE_REPEAT});
  }
  batch_bundle.dyn_state.viewport = (Ogl_Rect){0,0,screen_dim.x,screen_dim.y};
  ogl_buf_update(&batch_bundle.ubos[0].buffer, 0, &m, 1, sizeof(Batch_Vertex));


  R2D *rend = arena_push_array(arena, R2D, 1);
  rend->arena = arena;

  // initialize the tex array
  rend->tex_array.count = 0;
  rend->tex_array.cap = REND_MAX_TEXTURES;
  rend->tex_array.textures = arena_push_array(arena, R2D_Tex_Array, rend->tex_array.cap);

  return rend;
}

void r2d_end(R2D *rend) {
  R2D_Quad_Array quads = rend_quad_chunk_list_to_array(rend->arena, &rend->list);
  R2D_Tex_Array textures = {};
  textures.count = 0;

  Batch_Vertex *batch_vertices = arena_push_array(rend->arena, Batch_Vertex,REND_MAX_INSTANCES);

  u64 vertex_idx  = 0;

  for (u64 quad_idx = 0; quad_idx < quads.count; ++quad_idx) {
    R2D_Quad *q = &quads.arr[quad_idx];

    s64 tex_idx = rend_tex_array_try_add(&rend->tex_array, q->tex);
    bool tex_added = (tex_idx >= 0);

    Batch_Vertex v = (Batch_Vertex){
      // TODO: make src_rect/dst_rect/color be Rend types! remove this aliasing! (@FIXME)
      .src_rect = *(v4*)&q->src_rect,
      .dst_rect = *(v4*)&q->dst_rect,
      .color = *(v4*)&q->color,
      .rot_rad = q->rot_rad,
      .tex_slot = tex_idx,
    };

    batch_vertices[vertex_idx] = v;
    vertex_idx+=1;

    if (vertex_idx > REND_MAX_INSTANCES || quad_idx+1 >= quads.count || !tex_added) {
      r2d_flush(rend, batch_vertices, vertex_idx);
      // cleanup
      rend_tex_array_clear(&rend->tex_array);
      vertex_idx = 0;
    }
  }

}

void r2d_push_quad(R2D *rend, R2D_Quad q) {
  rend_quad_chunk_list_push(rend->arena, &rend->list, 256, q);
}

