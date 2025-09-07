// clang -O3 $(pkg-config --cflags sdl3 glew) -o main main.c $(pkg-config --libs sdl3 glew) && ./main
// + sudo dnf install SDL3 SDL3-devel glew glew-devel
#include <SDL3/SDL.h>
#include <GL/glew.h>
#include "ogl.h"

// TODO: support WASM with COMPATIBILTY FLAG and App_X functions

#include <stdio.h>
#include <assert.h>

void game_init(void);
void game_update(float dt);
void game_render(void);

void sdl_err(const char *message) { fprintf(stderr, "%s: %s\n", message, SDL_GetError()); }
int main(int argc, char *argv[]) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    sdl_err("Could not initialize SDL");
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  // TODO(ilias): Maybe here we need to set the COMPATIBILITY profile
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

  SDL_Window *window = SDL_CreateWindow("OpenGL 4.3 Core Profile (Manual Load)", 800, 600, SDL_WINDOW_OPENGL);
  if (!window) {
    sdl_err("Could not create window");
  }

  SDL_GLContext context = SDL_GL_CreateContext(window);
  if (!context) {
    sdl_err("Could not create OpenGL context");
  }

  if (!SDL_GL_MakeCurrent(window, context)) {
    sdl_err("Could not make OpenGL context current");
  }
  glewInit();

  printf("OpenGL version: %s\n", glGetString(GL_VERSION));
  printf("GLSL version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

  SDL_GL_SetSwapInterval(1);
  game_init();

  bool running = true;
  SDL_Event event;
  while (running) {
    while (SDL_PollEvent(&event) != 0) {
      if (event.type == SDL_EVENT_QUIT) {
        running = 0;
      }
    }

    game_update(0);
    game_render();

    SDL_GL_SwapWindow(window);
  }

  SDL_GL_DestroyContext(context);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
