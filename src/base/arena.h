#ifndef _ARENA_H__
#define _ARENA_H__
#include "helper.h"
// TODO: Poison non-committed memory with Asan, if we got it, we should stub it I think, casey style

#define ARENA_DEFAULT_CHUNK_SIZE KB(4)

typedef struct {
  void *backing_memory;
  u64 alignment; //typically 64
  u64 current;
  u64 reserved;
  u64 committed;
} Arena;
static_assert(sizeof(Arena) < ARENA_DEFAULT_CHUNK_SIZE);

static void arena_align_forward(Arena *arena) {
  arena->current = align_pow2(UINT_FROM_PTR(arena->backing_memory) + arena->current, arena->alignment) - UINT_FROM_PTR(arena->backing_memory);
}

static void arena_destroy(Arena *arena) {
  M_RELEASE(arena->backing_memory, arena->reserved);
}

static void* arena_push_nz(Arena *arena, u64 size_in_bytes) {
  // Check if current allocation fits inside whole arena, return nullptr, TODO: we should do the linked list of arenas here sometime
  u64 remaining_reserved_bytes = arena->reserved - arena->current;
  assert(size_in_bytes < remaining_reserved_bytes && "Allocation exceeding Arena reserved space");

  // First align forward to ensure allocation will be aligned
  arena_align_forward(arena);

  // Commit as many chunks needed (maybe 0)
  if (arena->current + size_in_bytes > arena->committed) {
    u64 extra_bytes_needed = size_in_bytes - (arena->committed - arena->current);
    u64 chunks_needed = (extra_bytes_needed/ARENA_DEFAULT_CHUNK_SIZE) + 1;
    M_COMMIT(PTR_FROM_UINT(UINT_FROM_PTR(arena->backing_memory) + arena->committed), chunks_needed*ARENA_DEFAULT_CHUNK_SIZE);
    arena->committed += chunks_needed * ARENA_DEFAULT_CHUNK_SIZE;
  }

  // Return chunk of memory, increment allocator to account for it
  void *ret = PTR_FROM_UINT(UINT_FROM_PTR(arena->backing_memory) + arena->current);
  arena->current += size_in_bytes;

  assert(arena->current < arena->committed && "Arena's current idx due to align forward has exceeded committed idx");
  
  return ret;
}

static void * arena_push(Arena *arena, u64 size_in_bytes) {
  void *bytes = arena_push_nz(arena, size_in_bytes);
  M_ZERO(bytes, size_in_bytes);
  return bytes;
}

// maybe we should use M_RELEASE(base, bytes) if we exceed a committed chunk boundary??
static void arena_pop(Arena *arena, u64 bytes_to_pop) {
  arena->current -= bytes_to_pop;
}

#define arena_push_array(arena, s, count) arena_push((arena), sizeof(s)*(count))
#define arena_push_array_nz(arena, s, count) arena_push_nz((arena), sizeof(s)*(count))

#define arena_push_struct(arena, s) arena_push_array(arena, s, 1)
#define arena_push_struct_nz(arena, s) arena_push_array_nz(arena, s, 1)


// TODO: Maybe we should also mem_release if we crossed committed chunk boundaries
static void arena_clear(Arena *arena) {
  arena->current = sizeof(Arena);
  arena_align_forward(arena);
}

static Arena* arena_make_with_alignment(u64 size_in_bytes, u64 alignment) {
  u64 size_to_alloc = (size_in_bytes < ARENA_DEFAULT_CHUNK_SIZE) ? ARENA_DEFAULT_CHUNK_SIZE : size_in_bytes;

  // Reserve the whole memory and Commit only the first chunk
  void *memory = M_RESERVE(size_to_alloc);
  M_COMMIT(memory, ARENA_DEFAULT_CHUNK_SIZE);

  Arena *arena = memory;
  arena->backing_memory = memory;
  arena->alignment = alignment;
  arena->reserved = size_to_alloc;
  arena->committed = ARENA_DEFAULT_CHUNK_SIZE;

  // Use our handy arena_push to advance the current idx and align forward
  arena_push_struct_nz(arena, Arena);

  // Return the arena
  return arena;
}

static Arena* arena_make(u64 size_in_bytes) {
  return arena_make_with_alignment(size_in_bytes, 64);
}

#if 0
  // Arena test
  Arena* test_arena = arena_make(MB(256));
  u32 elem_count = 500;

  f32 *my_floats = arena_push(test_arena, sizeof(float)*elem_count);
  assert(my_floats);
  for (u32 i = 0; i < elem_count; ++i) { my_floats[i] = (f32)i; }

  f32 *my_floats2 = arena_push(test_arena, sizeof(float)*elem_count*2);
  assert(my_floats2);
  for (u32 i = 0; i < elem_count*2; ++i) { my_floats2[i] = (f32)i; }

  Ogl_Render_Bundle *my_unused_render_bundle = arena_push_struct(test_arena, Ogl_Render_Bundle);
  assert(my_unused_render_bundle);

  arena_clear(test_arena);
  arena_destroy(test_arena);
#endif 


#endif
