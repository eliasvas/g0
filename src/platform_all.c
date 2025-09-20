#include <SDL3/SDL.h>
#include <SDL3/SDL_log.h>

#include <stdio.h>
#include <assert.h>

#include "game.h"
#include "helper.h"
#include "profiler.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <GL/glew.h>
#define OGL_IMPLEMENTATION
#include "ogl.h"

// we need this to port to WASM sadly, because WASM programs are event based, no main loops :(
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

typedef struct {
  u8 *data;
  u64 width;
  u64 height;
  // Always RGBA so no need for component count
} Platform_Image_Data;

Platform_Image_Data platform_load_image_bytes_as_rgba(const char *filepath) {
  Platform_Image_Data img_data = {};

  stbi_set_flip_vertically_on_load(true);
  int width, height, nrChannels;
  u8 *px_data = stbi_load(filepath, &width, &height, &nrChannels, STBI_rgb_alpha);

  img_data.width = width;
  img_data.height = height;
  img_data.data = px_data;

  return img_data;
}

typedef struct {
  SDL_Window *window;
  SDL_GLContext context;

  f64 dt;
  u64 frame_start;

  Game_State gs;
} SDL_State;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
  TIME_FUNC;

  profiler_begin();

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("Could not initialize SDL");
    return SDL_APP_FAILURE;
  }

  // FIXME: Why can't we do ES 3.0 on desktop mode? We should be able to
#if (ARCH_WASM64 || ARCH_WASM32)
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
#endif
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

  SDL_State *sdl_state= M_ALLOC(sizeof(SDL_State));
  M_ZERO_STRUCT(sdl_state);
  *appstate = sdl_state;

  // TODO: Maybe there should be some configuration file for stuff like this and profiling and stuff
#define DEFAULT_WIN_DIM_X 800
#define DEFAULT_WIN_DIM_Y 600

  sdl_state->window = SDL_CreateWindow("g0", DEFAULT_WIN_DIM_X, DEFAULT_WIN_DIM_Y, SDL_WINDOW_OPENGL);
  if (!sdl_state->window) {
    SDL_Log("Could not create window");
  }

  sdl_state->context = SDL_GL_CreateContext(sdl_state->window);
  if (!sdl_state->context) {
    SDL_Log("Could not create OpenGL context");
  }

  if (!SDL_GL_MakeCurrent(sdl_state->window, sdl_state->context)) {
    SDL_Log("Could not make OpenGL context current");
  }
  glewInit();

  SDL_Log("OpenGL version: %s\n", glGetString(GL_VERSION));
  SDL_Log("Renderer: %s\n", glGetString(GL_RENDERER));
  SDL_Log("GLSL version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

  SDL_SetWindowResizable(sdl_state->window, true);
  SDL_GL_SetSwapInterval(1);

  sdl_state->gs.persistent_arena = arena_make(GB(1));
  sdl_state->gs.frame_arena = arena_make(MB(256));
  sdl_state->gs.screen_dim = v2m(DEFAULT_WIN_DIM_X, DEFAULT_WIN_DIM_Y);
  game_init(&sdl_state->gs);

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  TIME_FUNC;
  SDL_State *sdl_state = (SDL_State*)appstate;

  if (event->type == SDL_EVENT_QUIT) {
      return SDL_APP_SUCCESS;
  } else if (event->type == SDL_EVENT_WINDOW_RESIZED) {
    // Update Game_State with new window dimensions
    sdl_state->gs.screen_dim = v2m(event->window.data1, event->window.data2);
  }

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
  TIME_FUNC;
  SDL_State *sdl_state = (SDL_State*)appstate;
  // Perform update and render
  game_update(&sdl_state->gs, sdl_state->dt);
  game_render(&sdl_state->gs);

  // Swap the window
  SDL_GL_SwapWindow(sdl_state->window);
  
  // Perform timings
  u64 frame_end = SDL_GetTicksNS();
  sdl_state->dt = (frame_end - sdl_state->frame_start) / 1000000000.0;
  //printf("fps=%f begin=%f end=%f\n", 1.0/sdl_state->dt, (f32)sdl_state->frame_start, (f32)frame_end);
  sdl_state->frame_start = SDL_GetTicksNS();

  // Clear the per-frame arena
  arena_clear(sdl_state->gs.frame_arena);

  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  profiler_end_and_print();

  SDL_State *sdl_state = (SDL_State*)appstate;
  SDL_Log("Quitting");
  SDL_GL_DestroyContext(sdl_state->context);
  SDL_DestroyWindow(sdl_state->window);
  SDL_Quit();
}

u64 platform_read_cpu_timer() {
  return SDL_GetPerformanceCounter();
}

u64 platform_read_cpu_freq() {
  return SDL_GetPerformanceFrequency();
}

f64 platform_get_time() {
  return (f64)SDL_GetTicksNS() / 1000000000.0;
}

