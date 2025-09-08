#ifndef OGL_H__
#define OGL_H__
#include "math3d.h"

#include <SDL3/SDL_log.h>

// To debug OpenGL error on the fly
//GLuint e; for (e = glGetError(); e != GL_NO_ERROR; e = glGetError()) { printf("Error: %d\n", e); }

// TODO: What about Uniform buffers inside Render_Bundles?
// TODO: Maybe all internal functions should be a separate layer, like create_shader and conversions and stuff
// TODO: Add a Usage field in vertex buffer create to specify STREAM/DYNAMIC/STATIC

// https://www.3dgep.com/forward-plus/

#include <stdio.h>
#include "helper.h"

typedef enum {
  OGL_PRIM_TYPE_POINT,
  OGL_PRIM_TYPE_LINE,
  OGL_PRIM_TYPE_LINE_STRIP,
  OGL_PRIM_TYPE_TRIANGLE,
  OGL_PRIM_TYPE_TRIANGLE_STRIP,
  OGL_PRIM_TYPE_TRIANGLE_FAN,
}Ogl_Prim_Type;

typedef enum {
  OGL_BUF_KIND_VERTEX,
  OGL_BUF_KIND_INDEX,
  OGL_BUF_KIND_UNIFORM,
}Ogl_Buf_Kind;

typedef struct {
  s64 bytes_per_elem;
  s64 count;
  Ogl_Buf_Kind kind;

  u32 impl_state;
} Ogl_Buf;

typedef enum {
  OGL_DATA_TYPE_FLOAT,
  OGL_DATA_TYPE_INT,
  OGL_DATA_TYPE_IVEC2,
  OGL_DATA_TYPE_IVEC3,
  OGL_DATA_TYPE_IVEC4,
  OGL_DATA_TYPE_VEC2,
  OGL_DATA_TYPE_VEC3,
  OGL_DATA_TYPE_VEC4,
  OGL_DATA_TYPE_MAT4,
} Ogl_Data_Type;

typedef struct {
  // Description
  u32 location;
  Ogl_Data_Type type;
  // Extra
  u64 offset;
  u32 stride;
  b32 instanced;
  // Extra Extra (could delete.. or do the default initialization macro thingy)
  b32 avail;
} Ogl_Vert_Attrib;


typedef struct {
#define OGL_MAX_ATTRIBS 16
  // This is kind-of a hack, we use Ogl_Vert_Attrib's and don't fill most fields
  Ogl_Vert_Attrib vattribs[OGL_MAX_ATTRIBS];

  u64 impl_state;
}Ogl_Shader;

// This is complete bullshit, WHY do I need to make a vao at startup on MODERN opengl??????
INLINE void ogl_init() {
  GLuint vao = 0;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
}

// TODO: maybe we should make this too just a render bundle huh?!
INLINE void ogl_clear(v4 color) {
  glClearColor(color.x, color.y, color.z, color.w);
  glClear(GL_COLOR_BUFFER_BIT);
}
 
INLINE s64 ogl_buf_count_bytes(Ogl_Buf *buf) {
  return buf->count * buf->bytes_per_elem;
}

INLINE GLuint ogl_to_gl_buf_kind(Ogl_Buf_Kind kind) {
  switch (kind) {
    case OGL_BUF_KIND_VERTEX:  return GL_ARRAY_BUFFER;
    case OGL_BUF_KIND_INDEX:   return GL_ELEMENT_ARRAY_BUFFER;
    case OGL_BUF_KIND_UNIFORM: return GL_UNIFORM_BUFFER;
    default: break;
  }
  return 0; // I dont like this
}

INLINE b32 ogl_buf_update(Ogl_Buf *buf, u64 offset, void *data, u32 count, u32 bytes_per_elem) {
  if (buf) {
    GLuint gl_buf_kind = ogl_to_gl_buf_kind(buf->kind);
    glBindBuffer(gl_buf_kind, buf->impl_state);
   
    if (data) {
      glBufferSubData(gl_buf_kind, 0, count*bytes_per_elem, data);
GLuint e; for (e = glGetError(); e != GL_NO_ERROR; e = glGetError()) { printf("Error: %d\n", e); }
    }
    glBindBuffer(gl_buf_kind, 0);
  }
  return (buf != NULL);
}

INLINE b32 ogl_buf_init(Ogl_Buf *buf, Ogl_Buf_Kind kind, void *data, u32 count, u32 bytes_per_elem) {
  if (buf) {
    buf->kind = kind;
    buf->count = count;
    buf->bytes_per_elem = bytes_per_elem;

    GLuint gl_buf_kind = ogl_to_gl_buf_kind(kind);
    glGenBuffers(1, &buf->impl_state);
    glBindBuffer(gl_buf_kind, buf->impl_state);
    glBufferData(gl_buf_kind, count*bytes_per_elem, data, GL_STATIC_DRAW); // if data == nil, buf will be zeroed
    glBindBuffer(gl_buf_kind, 0);
  }
  return (buf != NULL);
}

INLINE Ogl_Buf ogl_buf_make(Ogl_Buf_Kind kind, void *data, u32 count, u32 bytes_per_elem) {
  Ogl_Buf buf;
  assert(ogl_buf_init(&buf, kind, data, count, bytes_per_elem));
  return buf;
}

INLINE b32 ogl_buf_deinit(Ogl_Buf *buf) {
  if (buf && buf->impl_state) {
    glDeleteBuffers(1, &buf->impl_state);
  }
  return (buf != NULL);
}

INLINE u32 ogl_data_get_count(Ogl_Data_Type type) {
  switch (type) {
    case OGL_DATA_TYPE_FLOAT: return 1;
    case OGL_DATA_TYPE_INT:   return 1;
    case OGL_DATA_TYPE_IVEC2: return 2;
    case OGL_DATA_TYPE_IVEC3: return 3;
    case OGL_DATA_TYPE_IVEC4: return 4;
    case OGL_DATA_TYPE_VEC2:  return 2;
    case OGL_DATA_TYPE_VEC3:  return 3;
    case OGL_DATA_TYPE_VEC4:  return 4;
    case OGL_DATA_TYPE_MAT4:  return 16;
  }
}


INLINE Ogl_Data_Type ogl_data_type_from_gl(GLenum type) {
  switch (type) {
    case GL_FLOAT:      return OGL_DATA_TYPE_FLOAT;
    case GL_INT:        return OGL_DATA_TYPE_INT;
    case GL_INT_VEC2:   return OGL_DATA_TYPE_IVEC2;
    case GL_INT_VEC3:   return OGL_DATA_TYPE_IVEC3;
    case GL_INT_VEC4:   return OGL_DATA_TYPE_IVEC4;
    case GL_FLOAT_VEC2: return OGL_DATA_TYPE_VEC2;
    case GL_FLOAT_VEC3: return OGL_DATA_TYPE_VEC3;
    case GL_FLOAT_VEC4: return OGL_DATA_TYPE_VEC4;
    case GL_FLOAT_MAT4: return OGL_DATA_TYPE_MAT4;
  }
  return OGL_DATA_TYPE_FLOAT;
}

INLINE GLuint ogl_prim_type_to_gl_type(Ogl_Prim_Type prim) {
  switch (prim) {
    case OGL_PRIM_TYPE_POINT:          return GL_POINTS;
    case OGL_PRIM_TYPE_LINE:           return GL_LINES;
    case OGL_PRIM_TYPE_LINE_STRIP:     return GL_LINE_STRIP;
    case OGL_PRIM_TYPE_TRIANGLE:       return GL_TRIANGLES;
    case OGL_PRIM_TYPE_TRIANGLE_STRIP: return GL_TRIANGLE_STRIP;
    case OGL_PRIM_TYPE_TRIANGLE_FAN:   return GL_TRIANGLE_FAN;
  }
}



INLINE GLuint _gl_create_shader(const char* vertex_source, const char* fragment_source) {
  GLuint vs = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vs, 1, &vertex_source, NULL);
  glCompileShader(vs);
  int success;
  char info_log[512];
  glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vs, 512, NULL, info_log);
    fprintf(stderr, "Vertex shader compilation failed:\n%s\n", info_log);
    return 0;
  }

  GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fs, 1, &fragment_source, NULL);
  glCompileShader(fs);
  glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fs, 512, NULL, info_log);
    fprintf(stderr, "Fragment shader compilation failed:\n%s\n", info_log);
    return 0;
  }

  GLuint sp = glCreateProgram();
  glAttachShader(sp, vs);
  glAttachShader(sp, fs);
  glLinkProgram(sp);
  glGetProgramiv(sp, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(sp, 512, NULL, info_log);
    fprintf(stderr, "Shader program linking failed:\n%s\n", info_log);
    return 0;
  }

  glDeleteShader(vs);
  glDeleteShader(fs);

  return sp;
}

INLINE b32 ogl_vert_attrib_is_integral(Ogl_Vert_Attrib *attrib) {
  Ogl_Data_Type type = attrib->type;
  return (type == OGL_DATA_TYPE_INT || type == OGL_DATA_TYPE_IVEC2 || type == OGL_DATA_TYPE_IVEC3 || type == OGL_DATA_TYPE_IVEC4);
}

INLINE void ogl_shader_detect_vert_attribs(Ogl_Shader *shader) {
  char name_buf[256];
  GLint attrib_count;
  glGetProgramiv(shader->impl_state, GL_ACTIVE_ATTRIBUTES, &attrib_count);
  for (GLint i = 0; i < attrib_count; ++i) {
    GLenum type;
    GLint size;
    GLsizei length;
    glGetActiveAttrib(shader->impl_state, i, 256, &length, &size, &type, name_buf);
    GLint location = glGetAttribLocation(shader->impl_state, name_buf);
    if (location == -1) continue;

    // Fill the attrib slot, so we can know if the attrib is there at runtime!
    shader->vattribs[location] = (Ogl_Vert_Attrib){
      .location = location,
      .type = ogl_data_type_from_gl(type),
    };
  }
}


INLINE b32 ogl_shader_init(Ogl_Shader *shader, const char* vertex_source, const char* fragment_source) {
  shader->impl_state = _gl_create_shader(vertex_source, fragment_source);
  if (shader->impl_state){
    ogl_shader_detect_vert_attribs(shader);
  }
  return (shader->impl_state != 0);
}

INLINE Ogl_Shader ogl_shader_make(const char* vertex_source, const char* fragment_source) {
  Ogl_Shader shader = {0};
  assert(ogl_shader_init(&shader, vertex_source, fragment_source)); 
  return shader;
}

INLINE void ogl_shader_deinit(Ogl_Shader *shader) {
  if (shader) {
    glDeleteProgram(shader->impl_state);
    shader->impl_state = 0;
  }
}

typedef enum { 
  OGL_DYN_STATE_FLAG_SCISSOR    = (0x1 << 0),
  OGL_DYN_STATE_FLAG_DEPTH_TEST = (0x1 << 1),
} Ogl_Dyn_State_Flags;

typedef struct {
  v4 viewport; // TODO: Make this a RECT of some kind
  v4 scissor;  // TODO: Make this a RECT of some kind
  u64 flags;
  // TODO: add extra dynamic state (e.g blend mode / depth state)
}Ogl_Dyn_State;

typedef struct {
  Ogl_Shader sp;

  Ogl_Buf index_buffer;

  Ogl_Buf vertex_buffer;
  Ogl_Vert_Attrib vattribs[OGL_MAX_ATTRIBS];

  Ogl_Dyn_State dyn_state;
} Ogl_Render_Bundle;

INLINE void ogl_render_bundle_bind(Ogl_Render_Bundle *bundle) {
  // Bind the shader
  glUseProgram(bundle->sp.impl_state); 
  // Bind the index buffer
  glBindBuffer(ogl_to_gl_buf_kind(bundle->index_buffer.kind), bundle->index_buffer.impl_state);
  // Bind the vertex buffer(s) + set attributes
  glBindBuffer(ogl_to_gl_buf_kind(bundle->vertex_buffer.kind), bundle->vertex_buffer.impl_state);
  for (u64 vattr_idx = 0; vattr_idx < array_count(bundle->vattribs); ++vattr_idx) {
    Ogl_Vert_Attrib *attr = &bundle->vattribs[vattr_idx];
    if (attr->avail) {
      glEnableVertexAttribArray(vattr_idx);
      GLsizei stride = (attr->stride == 0) ? (bundle->vertex_buffer.bytes_per_elem) : (attr->stride);
      if (ogl_vert_attrib_is_integral(attr)) {
        glVertexAttribIPointer(vattr_idx, ogl_data_get_count(attr->type), GL_INT, stride, (void*)attr->offset);
      } else {
        glVertexAttribPointer(vattr_idx, ogl_data_get_count(attr->type), GL_FLOAT, GL_FALSE, stride, (void*)attr->offset);
      }
      glVertexAttribDivisor(vattr_idx, attr->instanced);
    } else {
      glDisableVertexAttribArray(vattr_idx);
    }
  }
  // Set the dynamic state
  v4 viewport = bundle->dyn_state.viewport;
  glViewport(viewport.x, viewport.y, viewport.z, viewport.w);
  if (bundle->dyn_state.flags & OGL_DYN_STATE_FLAG_SCISSOR) {
    glEnable(GL_SCISSOR_TEST);
    v4 scissor = bundle->dyn_state.scissor;
    glViewport(scissor.x, scissor.y, scissor.z, scissor.w);
  } else {
    glDisable(GL_SCISSOR_TEST);
  }
  if (bundle->dyn_state.flags & OGL_DYN_STATE_FLAG_DEPTH_TEST) {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
  } else {
    glDisable(GL_DEPTH_TEST);
  }
}

INLINE void ogl_render_bundle_draw(Ogl_Render_Bundle *bundle, Ogl_Prim_Type prim, u32 vertex_count, u32 instance_count) {
  ogl_render_bundle_bind(bundle);
  // first = 0? why? we need to enhance the API
  glDrawArraysInstanced(ogl_prim_type_to_gl_type(prim), 0, vertex_count, instance_count);
}

INLINE void ogl_render_bundle_draw_instanced(Ogl_Render_Bundle *bundle, Ogl_Prim_Type prim, u32 vertex_count, u32 indices_count, u32 instance_count) {
  ogl_render_bundle_bind(bundle);
  //glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const void * indices, GLsizei instancecount);
}

#endif
